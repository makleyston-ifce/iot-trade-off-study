#ifndef IOT_STUDY_COMMUNICATION_MESSAGE_SIZES_H
#define IOT_STUDY_COMMUNICATION_MESSAGE_SIZES_H

#include <cstdint>

namespace ns3 {

constexpr uint32_t TCP_CONNECT_SIZE = 60u;
constexpr uint32_t MQTT_CONNECT_SIZE = 24u;
constexpr uint32_t MQTT_CONNACK_SIZE = 4u;
constexpr uint32_t MQTT_PUBLISH_SIZE = 16u;
constexpr uint32_t MQTT_PUBACK_SIZE = 4u;
constexpr uint32_t COAP_CON_SIZE = 32u;
constexpr uint32_t COAP_ACK_SIZE = 16u;
constexpr uint32_t AMQP_PROTOCOL_HEADER_SIZE = 8u;
constexpr uint32_t AMQP_START_SIZE = 64u;
constexpr uint32_t AMQP_START_OK_SIZE = 48u;
constexpr uint32_t AMQP_TUNE_SIZE = 32u;
constexpr uint32_t AMQP_TUNE_OK_SIZE = 32u;
constexpr uint32_t AMQP_OPEN_SIZE = 32u;
constexpr uint32_t AMQP_OPEN_OK_SIZE = 32u;
constexpr uint32_t APPLICATION_DATA_SIZE = 0u;

} // namespace ns3

#endif // IOT_STUDY_COMMUNICATION_MESSAGE_SIZES_H
