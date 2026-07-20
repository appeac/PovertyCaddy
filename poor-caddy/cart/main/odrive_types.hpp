#pragma once
#include "poor_caddy_protocol/control.hpp"
#include <cstdint>
namespace cart {
struct AxisStatus {
  bool valid{};
  std::uint32_t axis_state{};
  std::uint64_t axis_error{}, motor_error{}, encoder_error{},
      controller_error{};
  float measured_velocity_turns_per_second{}, current_amps{};
  std::uint64_t updated_at_ms{};
};
struct ODriveStatus {
  AxisStatus axis0{}, axis1{};
  float dc_bus_voltage{};
  std::uint64_t bus_voltage_updated_at_ms{};
  std::uint32_t parse_error_count{}, response_timeout_count{};
  std::uint64_t snapshot_at_ms{};
  bool uart_healthy{};
};
enum class ODriveActionKind : std::uint8_t {
  Velocity,
  Idle,
  ClosedLoop,
  ClearErrors
};
struct ODriveCommand {
  ODriveActionKind kind{ODriveActionKind::Idle};
  poor_caddy::WheelVelocityTargets targets{};
  bool feed_watchdogs{};
  std::uint64_t created_at_ms{};
  std::uint32_t generation{};
};
} // namespace cart
