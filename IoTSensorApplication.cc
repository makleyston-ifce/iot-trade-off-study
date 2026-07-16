#include "IoTHeader.h"
#include "IoTSensorApplication.h"

#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/log.h"
#include "ns3/packet.h"
#include "ns3/socket.h"
#include "ns3/tcp-socket-factory.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/uinteger.h"
#include <iostream>

namespace {

const char*
CommunicationMessageTypeToString(ns3::CommunicationMessageType type)
{
  switch (type) {
  case ns3::CommunicationMessageType::TCP_CONNECT:
    return "TCP_CONNECT";
  case ns3::CommunicationMessageType::MQTT_CONNECT:
    return "MQTT_CONNECT";
  case ns3::CommunicationMessageType::MQTT_CONNACK:
    return "MQTT_CONNACK";
  case ns3::CommunicationMessageType::MQTT_PUBLISH:
    return "MQTT_PUBLISH";
  case ns3::CommunicationMessageType::MQTT_PUBACK:
    return "MQTT_PUBACK";
  case ns3::CommunicationMessageType::COAP_CON:
    return "COAP_CON";
  case ns3::CommunicationMessageType::COAP_ACK:
    return "COAP_ACK";
  case ns3::CommunicationMessageType::AMQP_PROTOCOL_HEADER:
    return "AMQP_PROTOCOL_HEADER";
  case ns3::CommunicationMessageType::AMQP_START:
    return "AMQP_START";
  case ns3::CommunicationMessageType::AMQP_START_OK:
    return "AMQP_START_OK";
  case ns3::CommunicationMessageType::AMQP_TUNE:
    return "AMQP_TUNE";
  case ns3::CommunicationMessageType::AMQP_TUNE_OK:
    return "AMQP_TUNE_OK";
  case ns3::CommunicationMessageType::AMQP_OPEN:
    return "AMQP_OPEN";
  case ns3::CommunicationMessageType::AMQP_OPEN_OK:
    return "AMQP_OPEN_OK";
  case ns3::CommunicationMessageType::APPLICATION_DATA:
    return "APPLICATION_DATA";
  }
  return "UNKNOWN";
}

} // namespace

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("IoTSensorApplication");

TypeId
IoTSensorApplication::GetTypeId()
{
  static TypeId tid = TypeId("ns3::IoTSensorApplication")
    .SetParent<Application>()
    .SetGroupName("Applications")
    .AddConstructor<IoTSensorApplication>()
    .AddAttribute("RemoteAddress",
                  "Gateway address for UDP packets.",
                  AddressValue(),
                  MakeAddressAccessor(&IoTSensorApplication::m_peerAddress),
                  MakeAddressChecker())
    .AddAttribute("PayloadSize",
                  "Size of each UDP packet payload in bytes.",
                  UintegerValue(256),
                  MakeUintegerAccessor(&IoTSensorApplication::m_payloadSize),
                  MakeUintegerChecker<uint32_t>())
    .AddAttribute("SendInterval",
                  "Interval between UDP packets.",
                  TimeValue(Seconds(1.0)),
                  MakeTimeAccessor(&IoTSensorApplication::m_sendInterval),
                  MakeTimeChecker());
  return tid;
}

IoTSensorApplication::IoTSensorApplication()
  : m_socket(nullptr),
    m_peerAddress(),
    m_sendEvent(),
    m_startupEvent(),
    m_handshakeEvent(),
    m_payloadSize(256),
    m_sendInterval(Seconds(1.0)),
    m_profile(GetCommunicationProfile(CommunicationProtocol::MQTT)),
    m_securityProfile(ns3::GetSecurityProfile(SecurityProtocol::NONE)),
    m_securityState(SecurityState::DISABLED),
    m_securityMessageIndex(0),
    m_handshakeMessages(),
    m_commState(CommunicationState::READY),
    m_startupMessages(),
    m_startupIndex(0),
    m_sequenceNumber(0),
    m_messagesSent(0),
    m_totalControlOverheadBytes(0),
    m_totalApplicationOverheadBytes(0),
    m_dataStarted(false)
{
}

IoTSensorApplication::~IoTSensorApplication()
{
  m_socket = nullptr;
}

void
IoTSensorApplication::SetRemote(Address peer)
{
  m_peerAddress = peer;
}

void
IoTSensorApplication::SetPayloadSize(uint32_t size)
{
  m_payloadSize = size;
}

void
IoTSensorApplication::SetSendInterval(Time interval)
{
  m_sendInterval = interval;
}

void
IoTSensorApplication::SetCommunicationProfile(const CommunicationProfile& profile)
{
  m_profile = profile;
}

void
IoTSensorApplication::SetSecurityProfile(const SecurityProfile& profile)
{
  m_securityProfile = profile;
}

SecurityProfile
IoTSensorApplication::GetSecurityProfile() const
{
  return m_securityProfile;
}

uint64_t
IoTSensorApplication::GetMessagesSent() const
{
  return m_messagesSent;
}

uint64_t
IoTSensorApplication::GetTotalControlOverheadBytes() const
{
  return m_totalControlOverheadBytes;
}

uint64_t
IoTSensorApplication::GetTotalApplicationOverheadBytes() const
{
  return m_totalApplicationOverheadBytes;
}

// void
// IoTSensorApplication::StartApplication()
// {
//   if (!m_socket) {
//     m_socket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
//     m_socket->Connect(m_peerAddress);
//   }

//   ScheduleNextSend();
// }

void
IoTSensorApplication::StartApplication()
{
    if (!m_socket)
    {
        TypeId socketFactory = m_profile.usesTcp ? TcpSocketFactory::GetTypeId()
                                                 : UdpSocketFactory::GetTypeId();
        m_socket = Socket::CreateSocket(GetNode(), socketFactory);
        m_socket->Connect(m_peerAddress);
    }

    if (!m_profile.startupMessages.empty()) {
        m_commState = CommunicationState::STARTUP;
        m_startupMessages = m_profile.startupMessages;
        m_startupIndex = 0;
        SendCommunicationMessage();
        return;
    }

    if (m_securityProfile.protocol == SecurityProtocol::NONE) {
        m_securityState = SecurityState::ESTABLISHED;
        m_dataStarted = true;
        std::cout << "Sending IoT data" << std::endl;
        SendPacket();
        return;
    }

    StartSecurityHandshake();
}

void
IoTSensorApplication::StopApplication()
{
  if (m_sendEvent.IsPending()) {
    Simulator::Cancel(m_sendEvent);
  }
  if (m_startupEvent.IsPending()) {
    Simulator::Cancel(m_startupEvent);
  }
  if (m_handshakeEvent.IsPending()) {
    Simulator::Cancel(m_handshakeEvent);
  }

  if (m_socket) {
    m_socket->Close();
    m_socket = nullptr;
  }
}

void
IoTSensorApplication::ScheduleNextSend()
{
  if (!m_sendEvent.IsExpired()) {
    return;
  }

  m_sendEvent = Simulator::Schedule(m_sendInterval, &IoTSensorApplication::SendPacket, this);
}

void
IoTSensorApplication::SendCommunicationMessage()
{
  if (m_startupIndex >= m_startupMessages.size()) {
    m_commState = CommunicationState::READY;

    if (m_securityProfile.protocol == SecurityProtocol::NONE) {
      m_securityState = SecurityState::ESTABLISHED;
      m_dataStarted = true;
      std::cout << "Sending IoT data" << std::endl;
      SendPacket();
      return;
    }

    StartSecurityHandshake();
    return;
  }

  CommunicationMessageType message = m_startupMessages[m_startupIndex];
  std::cout << CommunicationMessageTypeToString(message) << std::endl;

  m_totalControlOverheadBytes += GetCommunicationMessageSize(message);

  Ptr<Packet> packet = Create<Packet>();
  m_socket->Send(packet);

  ++m_startupIndex;
  m_startupEvent = Simulator::Schedule(MilliSeconds(50), &IoTSensorApplication::SendCommunicationMessage, this);
}

void
IoTSensorApplication::SendPacket()
{
  if (m_securityState != SecurityState::ESTABLISHED) {
    return;
  }

  uint32_t packetSize = m_payloadSize + m_profile.applicationHeaderBytes;
  m_totalApplicationOverheadBytes += m_profile.applicationHeaderBytes;
  Ptr<Packet> packet = Create<Packet>(packetSize);

  IoTHeader header;
  header.SetMagicNumber(IoTHeader::MAGIC_NUMBER);
  header.SetProtocolVersion(IoTHeader::PROTOCOL_VERSION);
  header.SetMessageType(IoTMessageType::DATA);
  header.SetSensorId(static_cast<uint32_t>(GetNode()->GetId()));
  header.SetSequenceNumber(++m_sequenceNumber);
  header.SetTimestamp(Simulator::Now());
  packet->AddHeader(header);

  m_socket->Send(packet);
  m_messagesSent++;
  ScheduleNextSend();
}

void
IoTSensorApplication::StartSecurityHandshake()
{
  m_securityState = SecurityState::HANDSHAKE;
  m_securityMessageIndex = 0;
  m_handshakeMessages.clear();

  if (m_securityProfile.protocol == SecurityProtocol::TLS) {
    m_handshakeMessages = {
      SecurityMessageType::CLIENT_HELLO,
      SecurityMessageType::SERVER_HELLO,
      SecurityMessageType::CERTIFICATE,
      SecurityMessageType::FINISHED};
  } else if (m_securityProfile.protocol == SecurityProtocol::MTLS) {
    m_handshakeMessages = {
      SecurityMessageType::CLIENT_HELLO,
      SecurityMessageType::SERVER_HELLO,
      SecurityMessageType::CERTIFICATE,
      SecurityMessageType::CERTIFICATE_REQUEST,
      SecurityMessageType::CLIENT_CERTIFICATE,
      SecurityMessageType::FINISHED};
  }

  std::cout << "Security handshake started" << std::endl;
  SendSecurityMessage();
}

void
IoTSensorApplication::SendSecurityMessage()
{
  if (m_securityMessageIndex >= m_handshakeMessages.size()) {
    CompleteSecurityHandshake();
    return;
  }

  SecurityMessageType message = m_handshakeMessages[m_securityMessageIndex];
  bool isSent = (message == SecurityMessageType::CLIENT_HELLO ||
                 message == SecurityMessageType::CLIENT_CERTIFICATE ||
                 message == SecurityMessageType::FINISHED);

  std::string name;
  switch (message) {
  case SecurityMessageType::CLIENT_HELLO:
    name = "CLIENT_HELLO";
    break;
  case SecurityMessageType::SERVER_HELLO:
    name = "SERVER_HELLO";
    break;
  case SecurityMessageType::CERTIFICATE:
    name = "CERTIFICATE";
    break;
  case SecurityMessageType::CERTIFICATE_REQUEST:
    name = "CERTIFICATE_REQUEST";
    break;
  case SecurityMessageType::CLIENT_CERTIFICATE:
    name = "CLIENT_CERTIFICATE";
    break;
  case SecurityMessageType::FINISHED:
    name = "FINISHED";
    break;
  }

  std::cout << name << " " << (isSent ? "sent" : "received") << std::endl;
  Ptr<Packet> packet = Create<Packet>(16);
  m_socket->Send(packet);

  m_securityMessageIndex++;
  if (m_securityMessageIndex < m_handshakeMessages.size()) {
    m_handshakeEvent = Simulator::Schedule(Seconds(0.05), &IoTSensorApplication::SendSecurityMessage, this);
  } else {
    m_handshakeEvent = Simulator::Schedule(Seconds(0.05), &IoTSensorApplication::CompleteSecurityHandshake, this);
  }
}

void
IoTSensorApplication::CompleteSecurityHandshake()
{
  m_securityState = SecurityState::ESTABLISHED;
  std::cout << "Security handshake completed" << std::endl;
  m_dataStarted = true;
  std::cout << "Sending IoT data" << std::endl;
  SendPacket();
}

} // namespace ns3
