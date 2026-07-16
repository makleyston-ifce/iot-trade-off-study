#include "IoTGatewayApplication.h"
#include "IoTHeader.h"

#include "ns3/address-utils.h"
#include "ns3/inet-socket-address.h"
#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"

#include <iostream>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("IoTGatewayApplication");

TypeId
IoTGatewayApplication::GetTypeId()
{
  static TypeId tid = TypeId("ns3::IoTGatewayApplication")
    .SetParent<Application>()
    .SetGroupName("Applications")
    .AddConstructor<IoTGatewayApplication>();
  return tid;
}

IoTGatewayApplication::IoTGatewayApplication()
  : m_messagesReceived(0),
    m_messagesLost(0),
    m_delaySumMs(0.0),
    m_sensorStates()
{
}

IoTGatewayApplication::~IoTGatewayApplication()
{
}

uint64_t
IoTGatewayApplication::GetMessagesReceived() const
{
  return m_messagesReceived;
}

uint64_t
IoTGatewayApplication::GetMessagesLost() const
{
  return m_messagesLost;
}

double
IoTGatewayApplication::GetAverageApplicationDelayMs() const
{
  if (m_messagesReceived == 0) {
    return 0.0;
  }
  return m_delaySumMs / static_cast<double>(m_messagesReceived);
}

void
IoTGatewayApplication::StartApplication()
{
  // No local UDP listener needed; the gateway application attaches to the
  // PacketSink trace in main.cc to observe received packets.
}

void
IoTGatewayApplication::StopApplication()
{
}

// void
// IoTGatewayApplication::HandlePacketTrace(Ptr<const Packet> packet,
//                                          const Address& from,
//                                          const Address& localAddress)
// {

//   if (packet->GetSize() < IoTHeader().GetSerializedSize()) {
//     std::cout << "Discarding non-IoT packet\n";
//     return;
//   }

//   Ptr<Packet> copy = packet->Copy();
//   IoTHeader header;

//   uint32_t removed = copy->RemoveHeader(header);
//   if (removed != header.GetSerializedSize() || !header.IsValid()) {
//     std::cout << "Discarding non-IoT packet\n";
//     return;
//   }

//   uint32_t sensorId = header.GetSensorId();
//   uint32_t sequenceNumber = header.GetSequenceNumber();
//   Time timestamp = header.GetTimestamp();
//   Time receiveTime = Simulator::Now();
//   double delayMs = receiveTime.GetSeconds() * 1000.0 - timestamp.GetSeconds() * 1000.0;

//   m_messagesReceived++;
//   m_delaySumMs += delayMs;
//   UpdateSequenceTracking(sensorId, sequenceNumber);
// }
void
IoTGatewayApplication::HandlePacketTrace(Ptr<const Packet> packet,
                                         const Address& from,
                                         const Address& localAddress)
{
    // Verifica se o pacote possui pelo menos o tamanho mínimo do cabeçalho IoT
    if (packet->GetSize() < IoTHeader().GetSerializedSize())
    {
        std::cout << "Discarding non-IoT packet\n";
        return;
    }

    Ptr<Packet> copy = packet->Copy();

    IoTHeader header;

    uint32_t removed = copy->RemoveHeader(header);

    if (removed != header.GetSerializedSize() || !header.IsValid())
    {
        std::cout << "Discarding non-IoT packet\n";
        return;
    }

    // DEBUG (temporário)
    // std::cout
    //     << "Sensor " << header.GetSensorId()
    //     << " Seq " << header.GetSequenceNumber()
    //     << std::endl;

    uint32_t sensorId = header.GetSensorId();
    uint32_t sequenceNumber = header.GetSequenceNumber();

    Time timestamp = header.GetTimestamp();
    Time receiveTime = Simulator::Now();

    double delayMs =
        (receiveTime - timestamp).GetSeconds() * 1000.0;

    m_messagesReceived++;
    m_delaySumMs += delayMs;

    UpdateSequenceTracking(sensorId, sequenceNumber);
}

// void
// IoTGatewayApplication::UpdateSequenceTracking(uint32_t sensorId, uint32_t sequenceNumber)
// {
//   auto it = m_sensorStates.find(sensorId);
//   if (it == m_sensorStates.end())
//   {
//     m_sensorStates[sensorId] = {sequenceNumber};
//     return;
//   }

//   if (sequenceNumber > it->second.lastSequenceNumber + 1)
//   {
//     m_messagesLost += sequenceNumber - it->second.lastSequenceNumber - 1;
//   }

//   it->second.lastSequenceNumber = sequenceNumber;
// }
void
IoTGatewayApplication::UpdateSequenceTracking(uint32_t sensorId,
                                              uint32_t sequenceNumber)
{
  auto it = m_sensorStates.find(sensorId);
  if (it == m_sensorStates.end())
  {
    m_sensorStates[sensorId] = {sequenceNumber};
    return;
  }

  uint32_t last = it->second.lastSequenceNumber;
  if (sequenceNumber > last + 1)
  {
    m_messagesLost += sequenceNumber - last - 1;
  }

  if (sequenceNumber >= last)
  {
    it->second.lastSequenceNumber = sequenceNumber;
  }
}

} // namespace ns3
