#ifndef IOT_STUDY_CPU_MODEL_H
#define IOT_STUDY_CPU_MODEL_H

#include "CpuProfile.h"

namespace ns3 {

class CpuModel {
public:
  CpuModel();

  void Initialize(const CpuProfile& profile);
  void ConsumeTransmission();
  void ConsumeProtocolOperation(double cost);
  void ConsumeSecurityOperation(double cost);

  double GetCpuConsumed() const;
  double GetTransmissionCost() const;
  double GetProtocolCost() const;
  double GetSecurityCost() const;

  void Reset();

private:
  double m_cpuConsumed;
  double m_transmissionCost;
  double m_protocolCost;
  double m_securityCost;
};

} // namespace ns3

#endif // IOT_STUDY_CPU_MODEL_H
