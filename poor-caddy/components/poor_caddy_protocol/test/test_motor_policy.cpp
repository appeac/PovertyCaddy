#include "poor_caddy_protocol/motor_policy.hpp"
#include <cassert>
using namespace poor_caddy;
static MotorFeedback good(std::uint64_t n, AxisState s = AxisState::Idle) {
  MotorFeedback f{};
  f.uart_healthy = true;
  f.left.valid = f.right.valid = true;
  f.left.state = f.right.state = s;
  f.left.updated_at_ms = f.right.updated_at_ms = f.bus_updated_at_ms = n;
  f.bus_millivolts = 24000;
  return f;
}
void testMotorPolicy() {
  MotorPolicyConfig c{};
  c.transition_timeout_ms = 100;
  MotorPolicy p(c);
  PolicyInput i{};
  i.now_ms = 1;
  i.feedback = good(1);
  assert(p.update(i).state == MotorState::Initializing);
  i.now_ms = 2;
  i.feedback = good(2);
  assert(p.update(i).state == MotorState::ReadyIdle);
  i.have_command = true;
  i.command = {SpeedLevel::Forward1, SteeringState::Straight,
               OperatingMode::Drive};
  i.command_at_ms = 3;
  i.now_ms = 3;
  assert(p.update(i).state == MotorState::EnablingClosedLoop);
  i.now_ms = 4;
  i.feedback = good(4, AxisState::ClosedLoop);
  assert(p.update(i).state == MotorState::Running);
  i.estop_active = true;
  i.now_ms = 5;
  auto o = p.update(i);
  assert(o.state == MotorState::EmergencyStopped &&
         o.fault == FaultCode::Estop);
  i.estop_active = false;
  i.recovery_requested = true;
  i.command.speed = SpeedLevel::Stopped;
  i.command_at_ms = 6;
  i.now_ms = 6;
  i.feedback = good(6);
  assert(p.update(i).state == MotorState::Recovering);
}
