#ifndef NET_UTILS_H
#define NET_UTILS_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include "../watchy/Watchy.h"

struct NetResponse {
    int httpCode = 0;
    String body;
    String error;
    // If present, value of the HTTP Location header (useful for redirects).
    String location;
    // Optional response metadata (useful for diagnosing unsupported compression).
    String contentType;
    String contentEncoding;
};

namespace NetUtils {

// Connect to WiFi using Watchy's helper; optional retries and timeout per attempt.
bool ensureWiFi(Watchy &watchy, uint8_t retries = 1, uint32_t attemptTimeoutMs = 8000);

// Perform a simple GET request. For HTTPS, use an insecure client (server certs
// not pinned on the device). Automatically sets timeout and closes WiFi/BT
// after the request when requested.
NetResponse httpGet(String &url,
                    uint32_t timeoutMs = 30000,
                    bool shutdownRadio = true);

// Perform a POST with application/x-www-form-urlencoded body.
// Useful for endpoints like html.duckduckgo.com/html/.
NetResponse httpPostForm(String &url,
                         const String &formBody,
                         const String &origin = String(),
                         const String &referer = String(),
                         const String &acceptLanguage = String(),
                         uint32_t timeoutMs = 30000,
                         bool shutdownRadio = true);

// Basic IPv4 dotted-quad validator: fills octets if valid.
bool parseIp(const String &ip, uint8_t out[4]);

// Uniform target editor used by network and browser apps. Controls:
// UP/DOWN: prev/next letter (starts on space)
// ACCEPT: append letter
// BACK: delete last letter (cancel if empty)
// ACCEPT twice while last letter is space: drop the space and finish.
// Allowed chars: space, a-z, 0-9, '-', '.'
bool editTarget(Watchy &watchy,
                char *buffer,
                size_t maxLen,
                const char *title,
                const char *defaultValue = nullptr);

// When true, the last call to editTarget() was aborted via a long-press BACK.
// Use consumeExitToMenuRequest() to detect-and-clear it.
bool consumeExitToMenuRequest();

// WHOIS (RFC 3912): TCP/43, query terminated by CRLF, server closes when done.
// Performs a simple two-step lookup: query whois.iana.org, follow a "refer:"/"whois:" line when present.
String whoisLookup(const String &target,
                   uint32_t timeoutMs = 8000,
                   size_t maxBytes = 48 * 1024);

// DNS (RFC 1034/1035): minimal UDP client.
// - If `name` is an IPv4 address, automatically performs a PTR lookup (in-addr.arpa).
// - Otherwise performs A (qtype=1) by default.
// Uses the currently configured WiFi DNS server when available, otherwise falls back to 1.1.1.1.
String dnsLookup(const String &name,
                 uint16_t qtype = 1,
                 uint32_t timeoutMs = 2000,
                 uint8_t attempts = 2);

// Simple ICMP-echo TTL-based traceroute (classic behavior; not RFC 1393 option-based).
// Returns a pointer to a static buffer containing newline-separated results.
char* traceroute(const String &host, uint8_t maxHops = 30, uint16_t timeoutMs = 3000);
} // namespace NetUtils

#endif // NET_UTILS_H
