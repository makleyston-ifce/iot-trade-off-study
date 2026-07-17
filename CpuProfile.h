#ifndef IOT_STUDY_CPU_PROFILE_H
#define IOT_STUDY_CPU_PROFILE_H

namespace ns3 {

struct CpuProfile {
  // CPU cost coefficients are not calibrated in this model.
  // The current implementation does not report CPU cost metrics.

  double transmissionCost = 0.0;
  double receptionCost = 0.0;

  double mqttCost = 0.0;
  double coapCost = 0.0;
  double amqpCost = 0.0;

  double tlsCost = 0.0;
  double mtlsCost = 0.0;
};

} // namespace ns3

#endif // IOT_STUDY_CPU_PROFILE_H
