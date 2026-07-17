#ifndef IOT_STUDY_IOT_GATEWAY_APPLICATION_H
#define IOT_STUDY_IOT_GATEWAY_APPLICATION_H

#include <map>

#include "ns3/application.h"
#include "ns3/address.h"
#include "ns3/ptr.h"
#include "ns3/nstime.h"

namespace ns3 {

class Packet;
class PacketSink;

class IoTGatewayApplication : public Application {
public:
  static TypeId GetTypeId();
  IoTGatewayApplication();
  virtual ~IoTGatewayApplication();

  uint64_t GetMessagesReceived() const;
  uint64_t GetMessagesLost() const;
  double GetAverageApplicationDelayMs() const;
  uint64_t GetTotalApplicationBytesReceived() const;

  void HandlePacketTrace(Ptr<const Packet> packet, const Address& from, const Address& localAddress);

private:
  virtual void StartApplication() override;
  virtual void StopApplication() override;

  void UpdateSequenceTracking(uint32_t sensorId, uint32_t sequenceNumber);

  uint64_t m_messagesReceived;
  uint64_t m_messagesLost;
  double m_delaySumMs;
  uint64_t m_totalApplicationBytesReceived;

  struct SensorState {
    uint32_t lastSequenceNumber;
  };

  std::map<uint32_t, SensorState> m_sensorStates;
};

} // namespace ns3

#endif // IOT_STUDY_IOT_GATEWAY_APPLICATION_H
