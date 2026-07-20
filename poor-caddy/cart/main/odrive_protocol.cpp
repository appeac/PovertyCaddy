#include "odrive_protocol.hpp"
#include <cerrno>
#include <cmath>
#include <cstdio>
#include <cstdlib>
namespace cart::odrive_protocol {
static bool fit(int n, std::size_t z) {
  return n > 0 && static_cast<std::size_t>(n) < z;
}
bool formatVelocity(char *b, std::size_t n, std::uint8_t a, float v) {
  return b && std::isfinite(v) && a < 2 &&
         fit(std::snprintf(b, n, "v %u %.6f 0\n", a, static_cast<double>(v)),
             n);
}
bool formatRequestedState(char *b, std::size_t n, std::uint8_t a,
                          std::uint32_t s) {
  return b && a < 2 &&
         fit(std::snprintf(b, n, "w axis%u.requested_state %lu\n", a,
                           static_cast<unsigned long>(s)),
             n);
}
bool formatRead(char *b, std::size_t n, const char *p) {
  return b && p && fit(std::snprintf(b, n, "r %s\n", p), n);
}
bool formatWatchdog(char *b, std::size_t n, std::uint8_t a) {
  return b && a < 2 && fit(std::snprintf(b, n, "u %u\n", a), n);
}
bool formatClearErrors(char *b, std::size_t n) {
  return b && fit(std::snprintf(b, n, "sc\n"), n);
}
static bool tail(char *e) {
  while (*e == ' ' || *e == '\t' || *e == '\r' || *e == '\n')
    ++e;
  return *e == '\0';
}
bool parseFloat(const char *s, float &o) {
  if (!s)
    return false;
  errno = 0;
  char *e = nullptr;
  float v = std::strtof(s, &e);
  if (e == s || errno || !tail(e) || !std::isfinite(v))
    return false;
  o = v;
  return true;
}
bool parseUint32(const char *s, std::uint32_t &o) {
  if (!s || *s == '-')
    return false;
  errno = 0;
  char *e = nullptr;
  auto v = std::strtoul(s, &e, 0);
  if (e == s || errno || !tail(e) || v > UINT32_MAX)
    return false;
  o = static_cast<std::uint32_t>(v);
  return true;
}
bool parseUint64(const char *s, std::uint64_t &o) {
  if (!s || *s == '-')
    return false;
  errno = 0;
  char *e = nullptr;
  auto v = std::strtoull(s, &e, 0);
  if (e == s || errno || !tail(e))
    return false;
  o = static_cast<std::uint64_t>(v);
  return true;
}
} // namespace cart::odrive_protocol
