#pragma once
#include "poor_caddy_protocol/motor_policy.hpp"
#include <array>
#include <cstdint>
namespace cart_config {
constexpr bool kCommissioningModeEnabled = false;
constexpr poor_caddy::VelocityMilliTurnsPerSecond kCommissioningMaxVelocity =
    250;
constexpr std::uint32_t kCommissioningLeaseMs = 500;
constexpr int kODriveUartPort = 1, kODriveTxGpio = 2, kODriveRxGpio = 3,
              kEstopSenseGpio = 8;
constexpr bool kEstopActiveLow = true;
constexpr std::uint32_t kODriveBaud = 115200, kUartResponseTimeoutMs = 50,
                        kCommandMaxAgeMs = 100, kCommandPeriodMs = 10,
                        kTelemetryPeriodMs = 100, kDiagnosticPeriodMs = 500,
                        kWatchdogFeedPeriodMs = 100;
constexpr std::size_t kMaxLineLength = 96;
constexpr std::array<std::uint8_t, 6> kWearableMac{{0x24, 0x6F, 0x28, 0, 0, 1}};
constexpr std::array<std::uint8_t, 16> kPmk{{'R', 'E', 'P', 'L', 'A', 'C', 'E',
                                             '_', 'P', 'M', 'K', '_', '1', '6',
                                             'B', '!'}};
constexpr std::array<std::uint8_t, 16> kLmk{{'R', 'E', 'P', 'L', 'A', 'C', 'E',
                                             '_', 'L', 'M', 'K', '_', '1', '6',
                                             'B', '!'}};
inline poor_caddy::MotorPolicyConfig motorPolicy() {
  poor_caddy::MotorPolicyConfig c{};
  c.drive.left_sign = 1;
  c.drive.right_sign = -1;
  c.drive.speed1 = 750;
  c.drive.speed2 = 1500;
  c.drive.speed3 = 2250;
  c.drive.maximum = 2500;
  c.min_bus_millivolts = 18000;
  c.critical_bus_millivolts = 54000;
  c.max_current_milliamps = 20000;
  return c;
}
} // namespace cart_config
