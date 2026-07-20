#include "poor_caddy_protocol/protocol.hpp"
#include "poor_caddy_protocol/sequence.hpp"
#include "poor_caddy_protocol/session_tracker.hpp"
#include <cassert>
using namespace poor_caddy;
void testProtocol() {
  const char *s = "123456789";
  assert(crc32(reinterpret_cast<const std::uint8_t *>(s), 9) == 0xCBF43926U);
  ControlState c{SpeedLevel::Forward2, SteeringState::Left,
                 OperatingMode::Hold};
  auto b = encodeControlPacket(7, 9, c, 3);
  ControlPacket p{};
  assert(decodeControlPacket(b.data(), b.size(), p) == DecodeResult::Valid);
  assert(p.control.mode == OperatingMode::Hold && p.flags == 3);
  assert(decodeControlPacket(b.data(), 23, p) == DecodeResult::WrongLength);
  auto bad = b;
  bad[0] ^= 1;
  assert(decodeControlPacket(bad.data(), bad.size(), p) ==
         DecodeResult::WrongMagic);
  bad = b;
  bad[19] = 1;
  bad[20] = bad[21] = bad[22] = bad[23] = 0;
  auto crc = crc32(bad.data(), 20);
  for (int i = 0; i < 4; i++)
    bad[20 + i] = static_cast<std::uint8_t>(crc >> (8 * i));
  assert(decodeControlPacket(bad.data(), bad.size(), p) ==
         DecodeResult::ReservedNonzero);
  assert(classifySequence(true, UINT32_MAX, 0) == SequenceClass::Newer);
  assert(classifySequence(true, 4, 4) == SequenceClass::Duplicate);
  SessionTracker t;
  AcceptedCommand out{};
  assert(t.consider(1, 1, c, 1, false, out) ==
         ValidationResult::ConflictingSession);
  assert(t.consider(1, 2, c, 2, false, out) ==
         ValidationResult::ConflictingSession);
  assert(t.consider(1, 3, c, 3, false, out) == ValidationResult::Valid);
  assert(t.consider(1, 3, c, 4, false, out) ==
         ValidationResult::DuplicateSequence);
}
