#pragma once
#include <cstddef>
#include <cstdint>
namespace cart::odrive_protocol {
constexpr const char *kPinnedFirmware = "ODrive legacy firmware v0.5.6";
constexpr std::uint32_t kAxisIdle = 1, kAxisClosedLoop = 8;
bool formatVelocity(char *, std::size_t, std::uint8_t, float);
bool formatRequestedState(char *, std::size_t, std::uint8_t, std::uint32_t);
bool formatRead(char *, std::size_t, const char *);
bool formatWatchdog(char *, std::size_t, std::uint8_t);
bool formatClearErrors(char *, std::size_t);
bool parseFloat(const char *, float &);
bool parseUint32(const char *, std::uint32_t &);
bool parseUint64(const char *, std::uint64_t &);
} // namespace cart::odrive_protocol
