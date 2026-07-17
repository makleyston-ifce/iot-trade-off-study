#ifndef IOT_STUDY_SECURITY_MESSAGE_H
#define IOT_STUDY_SECURITY_MESSAGE_H

#include <cstdint>
#include <string>

namespace ns3 {

enum class SecurityMessageType {
  CLIENT_HELLO,
  SERVER_HELLO,
  ENCRYPTED_EXTENSIONS,
  CERTIFICATE,
  CERTIFICATE_REQUEST,
  CLIENT_CERTIFICATE,
  CERTIFICATE_VERIFY,
  FINISHED,
  APPLICATION_DATA,
  NEW_SESSION_TICKET
};

struct SecurityMessage {
  SecurityMessageType type;
  uint32_t headerSize;
  bool carriesPayload;
  std::string name;
};

// Typical TLS 1.3 handshake sizes, including the 5-byte TLS record header and
// handshake payload bytes where appropriate.
// RFC 8446
constexpr uint32_t TLS_CLIENT_HELLO_SIZE = 512; // RFC 8446 Section 4.1.2: typical ClientHello size including record header.
constexpr uint32_t TLS_SERVER_HELLO_SIZE = 128; // RFC 8446 Section 4.1.3: typical ServerHello size including record header.
constexpr uint32_t TLS_ENCRYPTED_EXTENSIONS_SIZE = 80; // RFC 8446 Section 4.1.4: typical EncryptedExtensions message size.
constexpr uint32_t TLS_CERTIFICATE_SIZE = 1200; // RFC 8446 Section 4.4.2: Certificate message often carries a 1 KB certificate chain.
constexpr uint32_t TLS_CERTIFICATE_REQUEST_SIZE = 64; // RFC 8446 Section 4.4.4: typical CertificateRequest size.
constexpr uint32_t TLS_CLIENT_CERTIFICATE_SIZE = 1200; // RFC 8446 Section 4.4.4: client certificate size similar to server certificate.
constexpr uint32_t TLS_CERTIFICATE_VERIFY_SIZE = 96; // RFC 8446 Section 4.4.3: signature and handshake headers.
constexpr uint32_t TLS_FINISHED_SIZE = 64; // RFC 8446 Section 4.4.4: Finished message with verify data.
constexpr uint32_t TLS_APPLICATION_DATA_OVERHEAD = 21; // RFC 8446 Section 5.2: 5-byte record header + 16-byte AEAD tag overhead.
constexpr uint32_t TLS_NEW_SESSION_TICKET_SIZE = 100; // RFC 8446 Section 4.6: typical NewSessionTicket message size.

} // namespace ns3

#endif // IOT_STUDY_SECURITY_MESSAGE_H
