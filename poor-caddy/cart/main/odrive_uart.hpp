#pragma once

#include "driver/uart.h"
#include <cstddef>
#include <cstdint>

namespace odrive {

enum class Axis : std::uint8_t { Left = 0, Right = 1 };
enum class RequestedState : std::uint8_t {
  Idle = 1,
  EncoderIndexSearch = 6,
  EncoderOffsetCalibration = 7,
  ClosedLoopControl = 8,
};

struct DualVelocity { float left_turns_per_second; float right_turns_per_second; };
struct AxisTelemetry { float velocity_turns_per_second{}; std::uint32_t axis_error{}; };
struct Telemetry { AxisTelemetry left{}; AxisTelemetry right{}; };
struct Errors {
  std::uint32_t axis{}, motor{}, encoder{}, controller{};
};
struct CommunicationHealth {
  bool healthy{};
  std::uint32_t consecutive_failures{};
  std::uint64_t last_success_ms{};
};

class Uart final {
 public:
  Uart(uart_port_t port, std::uint32_t response_timeout_ms,
       std::size_t maximum_line_length);

  // Verified against ODrive Robotics firmware v0.5.6 on ODrive v3.6 hardware.
  // ASCII grammar used here is precisely:
  //   v axisN <turns/s> 0                 (velocity and zero torque feed-forward)
  //   w axisN.requested_state <integer>   (state request)
  //   u axisN                             (watchdog feed)
  //   r axisN.encoder.vel_estimate        (telemetry)
  //   r axisN.{error|motor.error|encoder.error|controller.error} (errors)
  // N is 0 for the left axis and 1 for the right axis. Property paths differ
  // between ODrive releases; do not assume this mapping for another firmware.
  bool setVelocity(const DualVelocity& velocity);
  bool requestState(Axis axis, RequestedState state);
  bool feedWatchdog(Axis axis);
  bool pollTelemetry(Telemetry& telemetry);
  bool readErrors(Axis axis, Errors& errors);
  CommunicationHealth health() const { return health_; }

 private:
  bool writeLine(const char* format, ...);
  bool queryFloat(float& value, const char* path);
  bool queryUnsigned(std::uint32_t& value, const char* path);
  bool readLine(char* output, std::size_t capacity);
  void record(bool success);
  const char* axisName(Axis axis) const;

  uart_port_t port_;
  std::uint32_t timeout_ms_;
  std::size_t max_line_length_;
  CommunicationHealth health_{};
};

}  // namespace odrive
