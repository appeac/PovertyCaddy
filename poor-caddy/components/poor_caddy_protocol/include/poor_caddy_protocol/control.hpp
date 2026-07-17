#pragma once
#include <cstdint>

namespace poor_caddy {

enum class SpeedLevel : std::uint8_t { Stopped = 0, Forward1 = 1, Forward2 = 2, Forward3 = 3 };
enum class SteeringState : std::uint8_t { Straight = 0, Left = 1, Right = 2 };

struct ControlState { SpeedLevel speed{SpeedLevel::Stopped}; SteeringState steering{SteeringState::Straight}; };

constexpr bool isValidSpeed(std::uint8_t value) { return value <= static_cast<std::uint8_t>(SpeedLevel::Forward3); }
constexpr bool isValidSteering(std::uint8_t value) { return value <= static_cast<std::uint8_t>(SteeringState::Right); }
SpeedLevel speedUp(SpeedLevel current);
SpeedLevel speedDown(SpeedLevel current);

struct ButtonInputs { bool speed_up_pressed; bool speed_down_pressed; bool left_held; bool right_held; bool stop_held; };
ControlState resolveButtons(ControlState previous, ButtonInputs inputs);

struct DriveTargets { std::uint16_t base_pwm; std::uint16_t left_reduction; std::uint16_t right_reduction; };
struct WheelPwm { std::uint16_t left; std::uint16_t right; };
DriveTargets driveTargets(ControlState state);
WheelPwm wheelPwmFromTargets(DriveTargets targets);
}
