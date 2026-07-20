#include "poor_caddy_protocol/control.hpp"
#include "poor_caddy_protocol/ramp.hpp"
#include <cassert>
using namespace poor_caddy;
void testControl() {
  ControlState s{};
  s = resolveButtons(s, {true, false, false, false, false});
  assert(s.speed == SpeedLevel::Forward1);
  s = resolveButtons(s, {true, true, true, false, false});
  assert(s.speed == SpeedLevel::Stopped &&
         s.steering == SteeringState::Straight &&
         s.mode == OperatingMode::Recover);
  DriveConfig c{};
  c.left_sign = 1;
  c.right_sign = -1;
  c.speed2 = 2000;
  c.left_turn_inner_permille = 500;
  s = {SpeedLevel::Forward2, SteeringState::Left, OperatingMode::Drive};
  auto t = wheelVelocityTargets(s, c);
  assert(t.left_millturns_per_second == 1000 &&
         t.right_millturns_per_second == -2000);
  VelocityRamp r;
  r.reset();
  assert(r.update(1000, 5000, 1000, 2000) == 50);
  for (int i = 0; i < 19; i++)
    r.update(1000, 50, 1000, 2000);
  assert(r.value() == 1000);
  assert(r.update(0, 50, 1000, 2000) == 900);
}
