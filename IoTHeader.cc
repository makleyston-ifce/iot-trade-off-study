#include "IoTHeader.h"

#include "ns3/buffer.h"
#include "ns3/log.h"
#include "ns3/simulator.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("IoTHeader");
NS_OBJECT_ENSURE_REGISTERED(IoTHeader);

IoTHeader::IoTHeader()
  : m_magicNumber(MAGIC_NUMBER),
    m_protocolVersion(PROTOCOL_VERSION),
    m_messageType(IoTMessageType::DATA),
    m_sensorId(0),
    m_sequenceNumber(0),
    m_timestamp(Simulator::Now())
{
}

void
IoTHeader::SetMagicNumber(uint32_t magicNumber)
{
  m_magicNumber = magicNumber;
}

uint32_t
IoTHeader::GetMagicNumber() const
{
  return m_magicNumber;
}

void
IoTHeader::SetProtocolVersion(uint16_t protocolVersion)
{
  m_protocolVersion = protocolVersion;
}

uint16_t
IoTHeader::GetProtocolVersion() const
{
  return m_protocolVersion;
}

void
IoTHeader::SetMessageType(IoTMessageType messageType)
{
  m_messageType = messageType;
}

IoTMessageType
IoTHeader::GetMessageType() const
{
  return m_messageType;
}

void
IoTHeader::SetSensorId(uint32_t sensorId)
{
  m_sensorId = sensorId;
}

uint32_t
IoTHeader::GetSensorId() const
{
  return m_sensorId;
}

void
IoTHeader::SetSequenceNumber(uint32_t sequenceNumber)
{
  m_sequenceNumber = sequenceNumber;
}

uint32_t
IoTHeader::GetSequenceNumber() const
{
  return m_sequenceNumber;
}

void
IoTHeader::SetTimestamp(Time timestamp)
{
  m_timestamp = timestamp;
}

Time
IoTHeader::GetTimestamp() const
{
  return m_timestamp;
}

bool
IoTHeader::IsValid() const
{
  return m_magicNumber == MAGIC_NUMBER && m_protocolVersion == PROTOCOL_VERSION;
}

TypeId
IoTHeader::GetTypeId()
{
  static TypeId tid = TypeId("ns3::IoTHeader")
    .SetParent<Header>()
    .SetGroupName("Applications")
    .AddConstructor<IoTHeader>();
  return tid;
}

TypeId
IoTHeader::GetInstanceTypeId() const
{
  return GetTypeId();
}

void
IoTHeader::Print(std::ostream& os) const
{
  os << "(magic=0x" << std::hex << m_magicNumber << std::dec
     << " protocolVersion=" << m_protocolVersion
     << " messageType=" << static_cast<uint32_t>(m_messageType)
     << " sensorId=" << m_sensorId
     << " sequenceNumber=" << m_sequenceNumber
     << " timestamp=" << m_timestamp.As(Time::S) << ")";
}

uint32_t
IoTHeader::GetSerializedSize() const
{
  return 4 + 2 + 1 + 4 + 4 + 8;
}

void
IoTHeader::Serialize(Buffer::Iterator start) const
{
  Buffer::Iterator i = start;
  i.WriteHtonU32(m_magicNumber);
  i.WriteHtonU16(m_protocolVersion);
  i.WriteU8(static_cast<uint8_t>(m_messageType));
  i.WriteHtonU32(m_sensorId);
  i.WriteHtonU32(m_sequenceNumber);
  i.WriteHtonU64(m_timestamp.GetTimeStep());
}

uint32_t
IoTHeader::Deserialize(Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  m_magicNumber = i.ReadNtohU32();
  m_protocolVersion = i.ReadNtohU16();
  m_messageType = static_cast<IoTMessageType>(i.ReadU8());
  m_sensorId = i.ReadNtohU32();
  m_sequenceNumber = i.ReadNtohU32();
  m_timestamp = TimeStep(i.ReadNtohU64());
  return GetSerializedSize();
}

} // namespace ns3
