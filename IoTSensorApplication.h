#ifndef IOT_STUDY_IOT_SENSOR_APPLICATION_H
#define IOT_STUDY_IOT_SENSOR_APPLICATION_H

#include "ns3/application.h"
#include "ns3/address.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/nstime.h"

#include <vector>

#include "CommunicationProfile.h"
#include "CpuModel.h"
#include "CpuProfile.h"
#include "SecurityProfile.h"

namespace ns3 {

enum class SecurityState {
  DISABLED,
  HANDSHAKE,
  ESTABLISHED
};

enum class CommunicationState {
  STARTUP,
  READY
};

class Socket;

class IoTSensorApplication : public Application {
public:
  static TypeId GetTypeId();
  IoTSensorApplication();
  virtual ~IoTSensorApplication();

  void SetRemote(Address peer);
  void SetPayloadSize(uint32_t size);
  void SetSendInterval(Time interval);
  void SetCommunicationProfile(const CommunicationProfile& profile);
  void SetSecurityProfile(const SecurityProfile& profile);
  SecurityProfile GetSecurityProfile() const;
  uint64_t GetMessagesSent() const;
  uint64_t GetTotalControlOverheadBytes() const;
  uint64_t GetTotalApplicationOverheadBytes() const;
  uint64_t GetTotalSecurityOverheadBytes() const;
  uint32_t GetTotalSecurityMessages() const;
  double GetCpuConsumed() const;
  double GetProtocolCpuCost() const;
  double GetSecurityCpuCost() const;

private:
  virtual void StartApplication() override;
  virtual void StopApplication() override;

  void ScheduleNextSend();
  void SendPacket();
  void SendCommunicationMessage();
  void StartSecurityHandshake();
  void SendSecurityMessage();
  void CompleteSecurityHandshake();

  Ptr<Socket> m_socket;
  Address m_peerAddress;
  EventId m_sendEvent;
  EventId m_startupEvent;
  EventId m_handshakeEvent;
  uint32_t m_payloadSize;
  Time m_sendInterval;
  CommunicationProfile m_profile;
  SecurityProfile m_securityProfile;
  SecurityState m_securityState;
  uint32_t m_securityMessageIndex;
  std::vector<SecurityMessage> m_handshakeMessages;
  CommunicationState m_commState;
  std::vector<CommunicationMessageType> m_startupMessages;
  uint32_t m_startupIndex;
  uint32_t m_sequenceNumber;
  uint64_t m_messagesSent;
  uint64_t m_totalControlOverheadBytes;
  uint64_t m_totalApplicationOverheadBytes;
  uint64_t m_totalSecurityOverheadBytes;
  uint32_t m_totalSecurityMessages;
  CpuModel m_cpuModel;
  bool m_dataStarted;
};

} // namespace ns3

#endif // IOT_STUDY_IOT_SENSOR_APPLICATION_H
