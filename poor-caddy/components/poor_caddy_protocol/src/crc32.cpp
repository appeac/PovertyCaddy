#include "poor_caddy_protocol/protocol.hpp"
namespace poor_caddy {
std::uint32_t crc32(const std::uint8_t *data, std::size_t length) {
  std::uint32_t crc = 0xFFFFFFFFU;
  for (std::size_t i = 0; i < length; ++i) {
    crc ^= data[i];
    for (int b = 0; b < 8; ++b)
      crc = (crc & 1U) ? (crc >> 1) ^ 0xEDB88320U : (crc >> 1);
  }
  return ~crc;
}
} // namespace poor_caddy
