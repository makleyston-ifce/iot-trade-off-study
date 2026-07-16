#ifndef IOT_STUDY_SECURITY_PROFILE_H
#define IOT_STUDY_SECURITY_PROFILE_H

#include <stdexcept>
#include <string>

namespace ns3 {

enum class SecurityProtocol {
  NONE,
  TLS,
  MTLS
};

struct SecurityProfile {
  SecurityProtocol protocol = SecurityProtocol::NONE;
  bool requiresHandshake = false;
  bool mutualAuthentication = false;
};

inline SecurityProfile
GetSecurityProfile(SecurityProtocol protocol)
{
  SecurityProfile profile;
  profile.protocol = protocol;

  switch (protocol) {
  case SecurityProtocol::NONE:
    profile.requiresHandshake = false;
    profile.mutualAuthentication = false;
    break;
  case SecurityProtocol::TLS:
    profile.requiresHandshake = true;
    profile.mutualAuthentication = false;
    break;
  case SecurityProtocol::MTLS:
    profile.requiresHandshake = true;
    profile.mutualAuthentication = true;
    break;
  default:
    throw std::invalid_argument("Unsupported SecurityProtocol");
  }

  return profile;
}

inline std::string
SecurityProtocolToString(SecurityProtocol protocol)
{
  switch (protocol) {
  case SecurityProtocol::NONE:
    return "NONE";
  case SecurityProtocol::TLS:
    return "TLS";
  case SecurityProtocol::MTLS:
    return "MTLS";
  default:
    return "Unknown";
  }
}

} // namespace ns3

#endif // IOT_STUDY_SECURITY_PROFILE_H
