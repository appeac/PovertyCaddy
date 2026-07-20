#include "odrive_uart.hpp"

#include "esp_timer.h"
#include <cerrno>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace odrive {
namespace { constexpr std::size_t kCommandCapacity = 96; }

Uart::Uart(uart_port_t port, std::uint32_t response_timeout_ms,
           std::size_t maximum_line_length)
    : port_(port), timeout_ms_(response_timeout_ms),
      max_line_length_(maximum_line_length) {}

const char* Uart::axisName(Axis axis) const {
  return axis == Axis::Left ? "axis0" : "axis1";
}

void Uart::record(bool success) {
  if (success) {
    health_.healthy = true;
    health_.consecutive_failures = 0;
    health_.last_success_ms = esp_timer_get_time() / 1000ULL;
  } else {
    health_.healthy = false;
    ++health_.consecutive_failures;
  }
}

bool Uart::writeLine(const char* format, ...) {
  char command[kCommandCapacity];
  va_list args;
  va_start(args, format);
  const int length = std::vsnprintf(command, sizeof(command), format, args);
  va_end(args);
  if (length <= 0 || static_cast<std::size_t>(length) >= sizeof(command) ||
      static_cast<std::size_t>(length) > max_line_length_) {
    record(false);
    return false;
  }
  const int written = uart_write_bytes(port_, command, length);
  const bool success = written == length;
  record(success);
  return success;
}

bool Uart::setVelocity(const DualVelocity& velocity) {
  // These writes deliberately remain adjacent: no other code owns this UART.
  return writeLine("v axis0 %.6g 0\n", static_cast<double>(velocity.left_turns_per_second)) &&
         writeLine("v axis1 %.6g 0\n", static_cast<double>(velocity.right_turns_per_second));
}

bool Uart::requestState(Axis axis, RequestedState state) {
  return writeLine("w %s.requested_state %u\n", axisName(axis),
                   static_cast<unsigned>(state));
}

bool Uart::feedWatchdog(Axis axis) { return writeLine("u %s\n", axisName(axis)); }

bool Uart::readLine(char* output, std::size_t capacity) {
  if (capacity < 2) return false;
  std::size_t used = 0;
  while (used + 1 < capacity && used < max_line_length_) {
    char byte{};
    const int count = uart_read_bytes(port_, &byte, 1, pdMS_TO_TICKS(timeout_ms_));
    if (count != 1) return false;
    if (byte == '\n') { output[used] = '\0'; return used != 0; }
    if (byte != '\r') output[used++] = byte;
  }
  return false;
}

bool Uart::queryFloat(float& value, const char* path) {
  if (!writeLine("r %s\n", path)) return false;
  char line[kCommandCapacity];
  if (!readLine(line, sizeof(line))) { record(false); return false; }
  char* end = nullptr;
  errno = 0;
  const float parsed = std::strtof(line, &end);
  const bool ok = errno == 0 && end != line && *end == '\0';
  if (ok) value = parsed;
  record(ok);
  return ok;
}

bool Uart::queryUnsigned(std::uint32_t& value, const char* path) {
  if (!writeLine("r %s\n", path)) return false;
  char line[kCommandCapacity];
  if (!readLine(line, sizeof(line))) { record(false); return false; }
  char* end = nullptr;
  errno = 0;
  const unsigned long parsed = std::strtoul(line, &end, 0);
  const bool ok = errno == 0 && end != line && *end == '\0';
  if (ok) value = static_cast<std::uint32_t>(parsed);
  record(ok);
  return ok;
}

bool Uart::pollTelemetry(Telemetry& telemetry) {
  return queryFloat(telemetry.left.velocity_turns_per_second, "axis0.encoder.vel_estimate") &&
         queryUnsigned(telemetry.left.axis_error, "axis0.error") &&
         queryFloat(telemetry.right.velocity_turns_per_second, "axis1.encoder.vel_estimate") &&
         queryUnsigned(telemetry.right.axis_error, "axis1.error");
}

bool Uart::readErrors(Axis axis, Errors& errors) {
  char path[48];
  auto read = [&](const char* suffix, std::uint32_t& value) {
    std::snprintf(path, sizeof(path), "%s.%s", axisName(axis), suffix);
    return queryUnsigned(value, path);
  };
  return read("error", errors.axis) && read("motor.error", errors.motor) &&
         read("encoder.error", errors.encoder) &&
         read("controller.error", errors.controller);
}

}  // namespace odrive
