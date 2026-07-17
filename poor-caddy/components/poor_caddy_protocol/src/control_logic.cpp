#include "poor_caddy_protocol/control.hpp"
#include <algorithm>

namespace poor_caddy {
SpeedLevel speedUp(SpeedLevel current) {
    switch (current) { case SpeedLevel::Stopped: return SpeedLevel::Forward1; case SpeedLevel::Forward1: return SpeedLevel::Forward2; case SpeedLevel::Forward2: return SpeedLevel::Forward3; case SpeedLevel::Forward3: return SpeedLevel::Forward3; }
    return SpeedLevel::Stopped;
}
SpeedLevel speedDown(SpeedLevel current) {
    switch (current) { case SpeedLevel::Forward3: return SpeedLevel::Forward2; case SpeedLevel::Forward2: return SpeedLevel::Forward1; case SpeedLevel::Forward1: return SpeedLevel::Stopped; case SpeedLevel::Stopped: return SpeedLevel::Stopped; }
    return SpeedLevel::Stopped;
}
ControlState resolveButtons(ControlState previous, ButtonInputs inputs) {
    ControlState next = previous;
    if (inputs.stop_held || (inputs.speed_up_pressed && inputs.speed_down_pressed)) next.speed = SpeedLevel::Stopped;
    else if (inputs.speed_up_pressed) next.speed = speedUp(next.speed);
    else if (inputs.speed_down_pressed) next.speed = speedDown(next.speed);
    if (next.speed == SpeedLevel::Stopped || (inputs.left_held && inputs.right_held) || (!inputs.left_held && !inputs.right_held)) next.steering = SteeringState::Straight;
    else if (inputs.left_held) next.steering = SteeringState::Left;
    else next.steering = SteeringState::Right;
    return next;
}
DriveTargets driveTargets(ControlState state) {
    switch (state.speed) {
    case SpeedLevel::Forward1: return {250, state.steering == SteeringState::Left ? static_cast<std::uint16_t>(150) : static_cast<std::uint16_t>(0), state.steering == SteeringState::Right ? static_cast<std::uint16_t>(150) : static_cast<std::uint16_t>(0)};
    case SpeedLevel::Forward2: return {500, state.steering == SteeringState::Left ? static_cast<std::uint16_t>(250) : static_cast<std::uint16_t>(0), state.steering == SteeringState::Right ? static_cast<std::uint16_t>(250) : static_cast<std::uint16_t>(0)};
    case SpeedLevel::Forward3: return {750, state.steering == SteeringState::Left ? static_cast<std::uint16_t>(150) : static_cast<std::uint16_t>(0), state.steering == SteeringState::Right ? static_cast<std::uint16_t>(150) : static_cast<std::uint16_t>(0)};
    case SpeedLevel::Stopped: return {0, 0, 0};
    }
    return {0,0,0};
}
WheelPwm wheelPwmFromTargets(DriveTargets t) { return {static_cast<std::uint16_t>(t.base_pwm > t.left_reduction ? t.base_pwm - t.left_reduction : 0), static_cast<std::uint16_t>(t.base_pwm > t.right_reduction ? t.base_pwm - t.right_reduction : 0)}; }
}
