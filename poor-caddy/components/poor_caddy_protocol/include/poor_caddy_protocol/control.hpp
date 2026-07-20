#pragma once
#include <cstdint>
namespace poor_caddy {
using VelocityMilliTurnsPerSecond = std::int32_t;
enum class SpeedLevel : std::uint8_t {
  Stopped = 0,
  Forward1 = 1,
  Forward2 = 2,
  Forward3 = 3
};
enum class SteeringState : std::uint8_t { Straight = 0, Left = 1, Right = 2 };
enum class OperatingMode : std::uint8_t {
  Drive = 0,
  Hold = 1,
  ManualPush = 2,
  Recover = 3
};
struct ControlState {
  SpeedLevel speed{SpeedLevel::Stopped};
  SteeringState steering{SteeringState::Straight};
  OperatingMode mode{OperatingMode::Drive};
};
struct ButtonInputs {
  bool speed_up_pressed;
  bool speed_down_pressed;
  bool left_held;
  bool right_held;
  bool stop_held;
};
struct DriveConfig {
  VelocityMilliTurnsPerSecond speed1{1000}, speed2{2000}, speed3{3000},
      maximum{3500};
  std::uint16_t left_turn_inner_permille{600}, right_turn_inner_permille{600};
  std::int8_t left_sign{1}, right_sign{1};
  std::uint16_t left_scale_permille{1000}, right_scale_permille{1000};
};
struct WheelVelocityTargets {
  VelocityMilliTurnsPerSecond left_millturns_per_second{};
  VelocityMilliTurnsPerSecond right_millturns_per_second{};
};
constexpr bool isValidSpeed(std::uint8_t v) { return v <= 3; }
constexpr bool isValidSteering(std::uint8_t v) { return v <= 2; }
constexpr bool isValidMode(std::uint8_t v) { return v <= 3; }
SpeedLevel speedUp(SpeedLevel);
SpeedLevel speedDown(SpeedLevel);
ControlState resolveButtons(ControlState, ButtonInputs);
WheelVelocityTargets wheelVelocityTargets(ControlState, const DriveConfig &);
} // namespace poor_caddy
