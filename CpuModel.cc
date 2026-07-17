#include "CpuModel.h"

namespace ns3 {

CpuModel::CpuModel()
  : m_cpuConsumed(0.0),
    m_transmissionCost(0.0),
    m_protocolCost(0.0),
    m_securityCost(0.0)
{
}

void
CpuModel::Initialize(const CpuProfile& profile)
{
  m_transmissionCost = profile.transmissionCost;
  m_protocolCost = 0.0;
  m_securityCost = 0.0;
  m_cpuConsumed = 0.0;
}

void
CpuModel::ConsumeTransmission()
{
  m_cpuConsumed += m_transmissionCost;
}

void
CpuModel::ConsumeProtocolOperation(double cost)
{
  m_protocolCost += cost;
  m_cpuConsumed += cost;
}

void
CpuModel::ConsumeSecurityOperation(double cost)
{
  m_securityCost += cost;
  m_cpuConsumed += cost;
}

double
CpuModel::GetCpuConsumed() const
{
  return m_cpuConsumed;
}

double
CpuModel::GetTransmissionCost() const
{
  return m_transmissionCost;
}

double
CpuModel::GetProtocolCost() const
{
  return m_protocolCost;
}

double
CpuModel::GetSecurityCost() const
{
  return m_securityCost;
}

void
CpuModel::Reset()
{
  m_cpuConsumed = 0.0;
  m_protocolCost = 0.0;
  m_securityCost = 0.0;
}

} // namespace ns3
