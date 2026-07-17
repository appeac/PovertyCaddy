#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include "poor_caddy_protocol/control.hpp"
namespace poor_caddy {
constexpr std::uint32_t kProtocolMagic = 0x50434459U; // PCDY
constexpr std::uint8_t kProtocolVersion = 1;
constexpr std::size_t kControlPacketSize = 24;
struct ControlPacket { std::uint32_t magic; std::uint32_t session_id; std::uint32_t sequence; std::uint8_t protocol_version; std::uint8_t desired_speed; std::uint8_t desired_steering; std::uint8_t flags; std::uint32_t reserved; std::uint32_t crc32; };
static_assert(sizeof(ControlPacket) == 24, "conceptual packet should remain 24 bytes on supported ABI; wire encoding is explicit");
enum class DecodeResult : std::uint8_t { Valid, WrongLength, WrongMagic, WrongVersion, BadCrc, InvalidSpeed, InvalidSteering };
using PacketBytes = std::array<std::uint8_t, kControlPacketSize>;
std::uint32_t crc32(const std::uint8_t* data, std::size_t length);
PacketBytes encodeControlPacket(std::uint32_t session_id, std::uint32_t sequence, ControlState state, std::uint8_t flags = 0);
DecodeResult decodeControlPacket(const std::uint8_t* data, std::size_t length, ControlPacket& out);
}
