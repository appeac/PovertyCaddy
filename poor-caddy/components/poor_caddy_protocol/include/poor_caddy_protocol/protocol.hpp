#pragma once
#include "poor_caddy_protocol/control.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
namespace poor_caddy {
constexpr std::uint32_t kProtocolMagic = 0x50434459U;
constexpr std::uint8_t kProtocolVersion = 2;
constexpr std::size_t kControlPacketSize = 24;
struct ControlPacket {
  std::uint32_t session_id{}, sequence{};
  ControlState control{};
  std::uint8_t flags{};
};
enum class DecodeResult : std::uint8_t {
  Valid,
  WrongLength,
  WrongMagic,
  WrongVersion,
  BadCrc,
  InvalidSpeed,
  InvalidSteering,
  InvalidMode,
  ReservedNonzero
};
using PacketBytes = std::array<std::uint8_t, kControlPacketSize>;
std::uint32_t crc32(const std::uint8_t *, std::size_t);
PacketBytes encodeControlPacket(std::uint32_t, std::uint32_t, ControlState,
                                std::uint8_t flags = 0);
DecodeResult decodeControlPacket(const std::uint8_t *, std::size_t,
                                 ControlPacket &);
} // namespace poor_caddy
