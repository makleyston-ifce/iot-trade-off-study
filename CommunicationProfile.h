#ifndef IOT_STUDY_COMMUNICATION_PROFILE_H
#define IOT_STUDY_COMMUNICATION_PROFILE_H

#include <stdexcept>
#include <string>
#include <vector>

#include "CommunicationMessageSizes.h"

namespace ns3 {

enum class CommunicationProtocol {
  MQTT,
  COAP,
  AMQP
};

enum class CommunicationMessageType {
  TCP_CONNECT,
  MQTT_CONNECT,
  MQTT_CONNACK,
  MQTT_PUBLISH,
  MQTT_PUBACK,
  COAP_CON,
  COAP_ACK,
  AMQP_PROTOCOL_HEADER,
  AMQP_START,
  AMQP_START_OK,
  AMQP_TUNE,
  AMQP_TUNE_OK,
  AMQP_OPEN,
  AMQP_OPEN_OK,
  APPLICATION_DATA
};

struct CommunicationProfile {
  CommunicationProtocol protocol = CommunicationProtocol::MQTT;
  bool usesTcp = false;
  bool persistentSession = false;
  bool keepAlive = false;
  uint32_t applicationHeaderBytes = 0;
  uint32_t controlPacketBytes = 0;
  std::vector<CommunicationMessageType> startupMessages;
};

inline uint32_t
GetCommunicationMessageSize(CommunicationMessageType message)
{
  switch (message) {
  case CommunicationMessageType::TCP_CONNECT:
    return TCP_CONNECT_SIZE;
  case CommunicationMessageType::MQTT_CONNECT:
    return MQTT_CONNECT_SIZE;
  case CommunicationMessageType::MQTT_CONNACK:
    return MQTT_CONNACK_SIZE;
  case CommunicationMessageType::MQTT_PUBLISH:
    return MQTT_PUBLISH_SIZE;
  case CommunicationMessageType::MQTT_PUBACK:
    return MQTT_PUBACK_SIZE;
  case CommunicationMessageType::COAP_CON:
    return COAP_CON_SIZE;
  case CommunicationMessageType::COAP_ACK:
    return COAP_ACK_SIZE;
  case CommunicationMessageType::AMQP_PROTOCOL_HEADER:
    return AMQP_PROTOCOL_HEADER_SIZE;
  case CommunicationMessageType::AMQP_START:
    return AMQP_START_SIZE;
  case CommunicationMessageType::AMQP_START_OK:
    return AMQP_START_OK_SIZE;
  case CommunicationMessageType::AMQP_TUNE:
    return AMQP_TUNE_SIZE;
  case CommunicationMessageType::AMQP_TUNE_OK:
    return AMQP_TUNE_OK_SIZE;
  case CommunicationMessageType::AMQP_OPEN:
    return AMQP_OPEN_SIZE;
  case CommunicationMessageType::AMQP_OPEN_OK:
    return AMQP_OPEN_OK_SIZE;
  case CommunicationMessageType::APPLICATION_DATA:
    return APPLICATION_DATA_SIZE;
  default:
    return 0;
  }
}

inline CommunicationProfile
GetCommunicationProfile(CommunicationProtocol protocol)
{
  CommunicationProfile profile;
  profile.protocol = protocol;

  switch (protocol) {
  case CommunicationProtocol::MQTT:
    profile.usesTcp = true;
    profile.persistentSession = true;
    profile.keepAlive = true;
    profile.applicationHeaderBytes = 8;
    profile.controlPacketBytes = 2;
    profile.startupMessages = {
      CommunicationMessageType::TCP_CONNECT,
      CommunicationMessageType::MQTT_CONNECT,
      CommunicationMessageType::MQTT_CONNACK
    };
    break;
  case CommunicationProtocol::COAP:
    profile.usesTcp = false;
    profile.persistentSession = false;
    profile.keepAlive = false;
    profile.applicationHeaderBytes = 4;
    profile.controlPacketBytes = 4;
    profile.startupMessages = {
      CommunicationMessageType::COAP_CON,
      CommunicationMessageType::COAP_ACK
    };
    break;
  case CommunicationProtocol::AMQP:
    profile.usesTcp = true;
    profile.persistentSession = true;
    profile.keepAlive = true;
    profile.applicationHeaderBytes = 16;
    profile.controlPacketBytes = 8;
    profile.startupMessages = {
      CommunicationMessageType::TCP_CONNECT,
      CommunicationMessageType::AMQP_PROTOCOL_HEADER,
      CommunicationMessageType::AMQP_START,
      CommunicationMessageType::AMQP_START_OK,
      CommunicationMessageType::AMQP_TUNE,
      CommunicationMessageType::AMQP_TUNE_OK,
      CommunicationMessageType::AMQP_OPEN,
      CommunicationMessageType::AMQP_OPEN_OK
    };
    break;
  default:
    throw std::invalid_argument("Unsupported CommunicationProtocol");
  }

  return profile;
}

inline std::string
CommunicationProtocolToString(CommunicationProtocol protocol)
{
  switch (protocol) {
  case CommunicationProtocol::MQTT:
    return "MQTT";
  case CommunicationProtocol::COAP:
    return "CoAP";
  case CommunicationProtocol::AMQP:
    return "AMQP";
  default:
    return "Unknown";
  }
}

inline uint32_t
GetProtocolControlOverheadBytes(const CommunicationProfile& profile)
{
  uint32_t total = 0;
  for (CommunicationMessageType message : profile.startupMessages) {
    total += GetCommunicationMessageSize(message);
  }
  return total;
}

inline uint32_t
GetApplicationMessageOverheadBytes(const CommunicationProfile& profile)
{
  return profile.applicationHeaderBytes;
}

} // namespace ns3

#endif // IOT_STUDY_COMMUNICATION_PROFILE_H
