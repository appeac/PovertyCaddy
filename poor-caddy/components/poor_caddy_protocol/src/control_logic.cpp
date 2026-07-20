#include "poor_caddy_protocol/control.hpp"
#include <algorithm>
#include <cstdint>
namespace poor_caddy {
SpeedLevel speedUp(SpeedLevel v) {
  return v == SpeedLevel::Stopped    ? SpeedLevel::Forward1
         : v == SpeedLevel::Forward1 ? SpeedLevel::Forward2
                                     : SpeedLevel::Forward3;
}
SpeedLevel speedDown(SpeedLevel v) {
  return v == SpeedLevel::Forward3   ? SpeedLevel::Forward2
         : v == SpeedLevel::Forward2 ? SpeedLevel::Forward1
                                     : SpeedLevel::Stopped;
}
ControlState resolveButtons(ControlState p, ButtonInputs i) {
  if (i.speed_up_pressed && i.speed_down_pressed) {
    p.speed = SpeedLevel::Stopped;
    p.mode = OperatingMode::Recover;
  } else if (i.stop_held) {
    p.speed = SpeedLevel::Stopped;
    p.mode = (i.left_held && !i.right_held) ? OperatingMode::Hold
                                            : OperatingMode::ManualPush;
  } else {
    if (i.speed_up_pressed)
      p.speed = speedUp(p.speed);
    else if (i.speed_down_pressed)
      p.speed = speedDown(p.speed);
    if (p.speed != SpeedLevel::Stopped)
      p.mode = OperatingMode::Drive;
  }
  if (p.speed == SpeedLevel::Stopped || (i.left_held == i.right_held))
    p.steering = SteeringState::Straight;
  else
    p.steering = i.left_held ? SteeringState::Left : SteeringState::Right;
  return p;
}
static VelocityMilliTurnsPerSecond clampScale(VelocityMilliTurnsPerSecond v,
                                              std::uint16_t scale,
                                              std::int8_t sign,
                                              VelocityMilliTurnsPerSecond max) {
  std::int64_t x = static_cast<std::int64_t>(v) * scale / 1000;
  x = std::min<std::int64_t>(x, max);
  return static_cast<VelocityMilliTurnsPerSecond>(x * (sign < 0 ? -1 : 1));
}
WheelVelocityTargets wheelVelocityTargets(ControlState s,
                                          const DriveConfig &c) {
  VelocityMilliTurnsPerSecond base =
      s.speed == SpeedLevel::Forward1   ? c.speed1
      : s.speed == SpeedLevel::Forward2 ? c.speed2
      : s.speed == SpeedLevel::Forward3 ? c.speed3
                                        : 0;
  base =
      std::clamp(base, static_cast<VelocityMilliTurnsPerSecond>(0), c.maximum);
  auto l = base, r = base;
  if (s.steering == SteeringState::Left)
    l = static_cast<VelocityMilliTurnsPerSecond>(
        static_cast<std::int64_t>(base) * c.left_turn_inner_permille / 1000);
  if (s.steering == SteeringState::Right)
    r = static_cast<VelocityMilliTurnsPerSecond>(
        static_cast<std::int64_t>(base) * c.right_turn_inner_permille / 1000);
  return {clampScale(l, c.left_scale_permille, c.left_sign, c.maximum),
          clampScale(r, c.right_scale_permille, c.right_sign, c.maximum)};
}
} // namespace poor_caddy
