#include "poor_caddy_protocol/protocol.hpp"
namespace poor_caddy {
static void put32(PacketBytes &b, std::size_t o, std::uint32_t v) {
  for (int i = 0; i < 4; i++)
    b[o + i] = static_cast<std::uint8_t>(v >> (8 * i));
}
static std::uint32_t get32(const std::uint8_t *b, std::size_t o) {
  return static_cast<std::uint32_t>(b[o]) |
         (static_cast<std::uint32_t>(b[o + 1]) << 8) |
         (static_cast<std::uint32_t>(b[o + 2]) << 16) |
         (static_cast<std::uint32_t>(b[o + 3]) << 24);
}
PacketBytes encodeControlPacket(std::uint32_t sid, std::uint32_t seq,
                                ControlState s, std::uint8_t flags) {
  PacketBytes b{};
  put32(b, 0, kProtocolMagic);
  put32(b, 4, sid);
  put32(b, 8, seq);
  b[12] = kProtocolVersion;
  b[13] = static_cast<std::uint8_t>(s.speed);
  b[14] = static_cast<std::uint8_t>(s.steering);
  b[15] = static_cast<std::uint8_t>(s.mode);
  b[16] = flags;
  put32(b, 20, crc32(b.data(), 20));
  return b;
}
DecodeResult decodeControlPacket(const std::uint8_t *d, std::size_t n,
                                 ControlPacket &p) {
  if (!d || n != kControlPacketSize)
    return DecodeResult::WrongLength;
  if (get32(d, 0) != kProtocolMagic)
    return DecodeResult::WrongMagic;
  if (d[12] != kProtocolVersion)
    return DecodeResult::WrongVersion;
  if (crc32(d, 20) != get32(d, 20))
    return DecodeResult::BadCrc;
  if (!isValidSpeed(d[13]))
    return DecodeResult::InvalidSpeed;
  if (!isValidSteering(d[14]))
    return DecodeResult::InvalidSteering;
  if (!isValidMode(d[15]))
    return DecodeResult::InvalidMode;
  if (d[17] || d[18] || d[19])
    return DecodeResult::ReservedNonzero;
  p.session_id = get32(d, 4);
  p.sequence = get32(d, 8);
  p.control = {static_cast<SpeedLevel>(d[13]),
               static_cast<SteeringState>(d[14]),
               static_cast<OperatingMode>(d[15])};
  p.flags = d[16];
  return DecodeResult::Valid;
}
} // namespace poor_caddy
