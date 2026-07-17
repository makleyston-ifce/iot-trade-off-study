#ifndef IOT_STUDY_CPU_PROFILE_H
#define IOT_STUDY_CPU_PROFILE_H

namespace ns3 {

struct CpuProfile {
  // PLACEHOLDER values for abstract CPU cost coefficients.
  // These will be calibrated later using literature or experimental data.

  double transmissionCost = 1.0; // PLACEHOLDER: cost per transmit operation.
  double receptionCost = 0.5;    // PLACEHOLDER: cost per receive operation.

  double mqttCost = 1.2;  // PLACEHOLDER: cost per MQTT protocol operation.
  double coapCost = 0.9;  // PLACEHOLDER: cost per CoAP protocol operation.
  double amqpCost = 1.5;  // PLACEHOLDER: cost per AMQP protocol operation.

  double tlsCost = 2.0;   // PLACEHOLDER: cost per TLS security operation.
  double mtlsCost = 2.5;  // PLACEHOLDER: cost per mTLS security operation.
};

} // namespace ns3

#endif // IOT_STUDY_CPU_PROFILE_H
