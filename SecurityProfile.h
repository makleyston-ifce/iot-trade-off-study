#ifndef IOT_STUDY_SECURITY_PROFILE_H
#define IOT_STUDY_SECURITY_PROFILE_H

#include <stdexcept>
#include <string>
#include <vector>

#include "SecurityMessage.h"

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

  std::vector<SecurityMessage> GetHandshakeMessages() const;
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

inline std::vector<SecurityMessage>
SecurityProfile::GetHandshakeMessages() const
{
  switch (protocol) {
  case SecurityProtocol::TLS:
    return {
      {SecurityMessageType::CLIENT_HELLO, TLS_CLIENT_HELLO_SIZE, false, "CLIENT_HELLO"},
      {SecurityMessageType::SERVER_HELLO, TLS_SERVER_HELLO_SIZE, false, "SERVER_HELLO"},
      {SecurityMessageType::ENCRYPTED_EXTENSIONS, TLS_ENCRYPTED_EXTENSIONS_SIZE, false, "ENCRYPTED_EXTENSIONS"},
      {SecurityMessageType::CERTIFICATE, TLS_CERTIFICATE_SIZE, false, "CERTIFICATE"},
      {SecurityMessageType::CERTIFICATE_VERIFY, TLS_CERTIFICATE_VERIFY_SIZE, false, "CERTIFICATE_VERIFY"},
      {SecurityMessageType::FINISHED, TLS_FINISHED_SIZE, false, "FINISHED"}
    };
  case SecurityProtocol::MTLS:
    return {
      {SecurityMessageType::CLIENT_HELLO, TLS_CLIENT_HELLO_SIZE, false, "CLIENT_HELLO"},
      {SecurityMessageType::SERVER_HELLO, TLS_SERVER_HELLO_SIZE, false, "SERVER_HELLO"},
      {SecurityMessageType::ENCRYPTED_EXTENSIONS, TLS_ENCRYPTED_EXTENSIONS_SIZE, false, "ENCRYPTED_EXTENSIONS"},
      {SecurityMessageType::CERTIFICATE, TLS_CERTIFICATE_SIZE, false, "CERTIFICATE"},
      {SecurityMessageType::CERTIFICATE_REQUEST, TLS_CERTIFICATE_REQUEST_SIZE, false, "CERTIFICATE_REQUEST"},
      {SecurityMessageType::CLIENT_CERTIFICATE, TLS_CLIENT_CERTIFICATE_SIZE, false, "CLIENT_CERTIFICATE"},
      {SecurityMessageType::CERTIFICATE_VERIFY, TLS_CERTIFICATE_VERIFY_SIZE, false, "CERTIFICATE_VERIFY"},
      {SecurityMessageType::FINISHED, TLS_FINISHED_SIZE, false, "FINISHED"}
    };
  case SecurityProtocol::NONE:
    return {};
  default:
    throw std::invalid_argument("Unsupported SecurityProtocol");
  }
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
