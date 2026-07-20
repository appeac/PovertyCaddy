#include "poor_caddy_protocol/control.hpp"
#include <algorithm>
#include <cstddef>

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
WheelVelocityTargets driveTargets(ControlState state, const DriveVelocityConfig& config) {
    if (state.speed == SpeedLevel::Stopped) return {0, 0};
    const auto index = static_cast<std::size_t>(state.speed) - 1U;
    if (index >= config.forward_speeds.size()) return {0, 0};

    const auto maximum = std::max<MilliTurnsPerSecond>(0, config.maximum_velocity);
    const auto outside = std::clamp(config.forward_speeds[index],
                                    static_cast<MilliTurnsPerSecond>(0), maximum);
    const auto ratio = std::min<std::uint16_t>(config.steering_ratios_milli[index], 1000U);
    const auto inside = static_cast<MilliTurnsPerSecond>(
        (static_cast<std::int64_t>(outside) * ratio) / 1000);

    MilliTurnsPerSecond left = outside;
    MilliTurnsPerSecond right = outside;
    if (state.steering == SteeringState::Left) left = inside;
    if (state.steering == SteeringState::Right) right = inside;
    return {left * (config.left_direction_sign < 0 ? -1 : 1),
            right * (config.right_direction_sign < 0 ? -1 : 1)};
}
}
