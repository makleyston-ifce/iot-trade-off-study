#ifndef IOT_STUDY_IOT_HEADER_H
#define IOT_STUDY_IOT_HEADER_H

#include "ns3/header.h"
#include "ns3/nstime.h"

#include <cstdint>
#include <ostream>

namespace ns3 {

enum class IoTMessageType : uint8_t {
  DATA = 0,
  CLIENT_HELLO,
  SERVER_HELLO,
  CERTIFICATE,
  CERTIFICATE_REQUEST,
  CLIENT_CERTIFICATE,
  FINISHED
};

class IoTHeader : public Header {
public:
  static constexpr uint32_t MAGIC_NUMBER = 0x494F5453;
  static constexpr uint16_t PROTOCOL_VERSION = 1;

  IoTHeader();

  void SetMagicNumber(uint32_t magicNumber);
  uint32_t GetMagicNumber() const;

  void SetProtocolVersion(uint16_t protocolVersion);
  uint16_t GetProtocolVersion() const;

  void SetMessageType(IoTMessageType messageType);
  IoTMessageType GetMessageType() const;

  void SetSensorId(uint32_t sensorId);
  uint32_t GetSensorId() const;

  void SetSequenceNumber(uint32_t sequenceNumber);
  uint32_t GetSequenceNumber() const;

  void SetTimestamp(Time timestamp);
  Time GetTimestamp() const;

  bool IsValid() const;

  static TypeId GetTypeId();
  TypeId GetInstanceTypeId() const override;
  void Print(std::ostream& os) const override;
  uint32_t GetSerializedSize() const override;
  void Serialize(Buffer::Iterator start) const override;
  uint32_t Deserialize(Buffer::Iterator start) override;

private:
  uint32_t m_magicNumber;
  uint16_t m_protocolVersion;
  IoTMessageType m_messageType;
  uint32_t m_sensorId;
  uint32_t m_sequenceNumber;
  Time m_timestamp;
};

} // namespace ns3

#endif // IOT_STUDY_IOT_HEADER_H
