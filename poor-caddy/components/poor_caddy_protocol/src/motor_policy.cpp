#include "poor_caddy_protocol/motor_policy.hpp"
#include <cstdlib>
namespace poor_caddy {
void MotorPolicy::enter(MotorState s, std::uint64_t n) {
  state_ = s;
  entered_at_ms_ = n;
  if (s != MotorState::ControlledStopping)
    stationary_at_ms_ = 0;
}
void MotorPolicy::latch(FaultCode f, std::uint64_t n) {
  if (fault_ == FaultCode::None)
    fault_ = f;
  enter(MotorState::Fault, n);
  left_ramp_.reset();
  right_ramp_.reset();
}
bool MotorPolicy::stationary(const PolicyInput &i) const {
  return std::llabs(i.feedback.left.velocity) <= config_.standstill_velocity &&
         std::llabs(i.feedback.right.velocity) <= config_.standstill_velocity;
}
bool MotorPolicy::healthy(const PolicyInput &i) const {
  const auto &f = i.feedback;
  return f.uart_healthy && f.left.valid && f.right.valid &&
         i.now_ms - f.left.updated_at_ms <= config_.telemetry_max_age_ms &&
         i.now_ms - f.right.updated_at_ms <= config_.telemetry_max_age_ms &&
         i.now_ms - f.bus_updated_at_ms <= config_.telemetry_max_age_ms &&
         f.left.errors == 0 && f.right.errors == 0 &&
         std::abs(f.left.current_milliamps) <= config_.max_current_milliamps &&
         std::abs(f.right.current_milliamps) <= config_.max_current_milliamps &&
         f.bus_millivolts >= config_.min_bus_millivolts &&
         f.bus_millivolts < config_.critical_bus_millivolts;
}
PolicyOutput MotorPolicy::update(const PolicyInput &i) {
  MotorAction a{};
  if (i.estop_active) {
    if (state_ != MotorState::EmergencyStopped) {
      fault_ = FaultCode::Estop;
      enter(MotorState::EmergencyStopped, i.now_ms);
    }
    a.request_idle = true;
    return {state_, fault_, a};
  }
  if (state_ != MotorState::BootSafe && state_ != MotorState::Initializing &&
      state_ != MotorState::EmergencyStopped && state_ != MotorState::Fault &&
      state_ != MotorState::Recovering && !healthy(i)) {
    latch(i.feedback.uart_healthy ? FaultCode::TelemetryStale : FaultCode::Uart,
          i.now_ms);
  }
  const bool fresh =
      i.have_command && i.now_ms - i.command_at_ms <= config_.link_timeout_ms;
  switch (state_) {
  case MotorState::BootSafe:
    a.request_idle = true;
    enter(MotorState::Initializing, i.now_ms);
    break;
  case MotorState::Initializing:
    a.request_idle = true;
    if (healthy(i) && i.feedback.left.state == AxisState::Idle &&
        i.feedback.right.state == AxisState::Idle)
      enter(MotorState::ReadyIdle, i.now_ms);
    else if (i.now_ms - entered_at_ms_ > config_.transition_timeout_ms)
      latch(FaultCode::TransitionTimeout, i.now_ms);
    break;
  case MotorState::ReadyIdle:
    a.request_idle = true;
    if (fresh && i.command.speed != SpeedLevel::Stopped) {
      left_ramp_.reset();
      right_ramp_.reset();
      enter(MotorState::EnablingClosedLoop, i.now_ms);
    }
    break;
  case MotorState::EnablingClosedLoop:
    a.request_closed_loop = true;
    if (i.feedback.left.state == AxisState::ClosedLoop &&
        i.feedback.right.state == AxisState::ClosedLoop)
      enter(MotorState::Running, i.now_ms);
    else if (i.now_ms - entered_at_ms_ > config_.transition_timeout_ms)
      latch(FaultCode::TransitionTimeout, i.now_ms);
    break;
  case MotorState::Running: {
    a.request_closed_loop = true;
    a.feed_watchdogs = true;
    if (!fresh || i.command.speed == SpeedLevel::Stopped) {
      enter(MotorState::ControlledStopping, i.now_ms);
      break;
    }
    auto t = wheelVelocityTargets(i.command, config_.drive);
    a.targets.left_millturns_per_second = left_ramp_.update(
        t.left_millturns_per_second, 5, config_.accel_units_per_second,
        config_.decel_units_per_second);
    a.targets.right_millturns_per_second = right_ramp_.update(
        t.right_millturns_per_second, 5, config_.accel_units_per_second,
        config_.decel_units_per_second);
    break;
  }
  case MotorState::ControlledStopping: {
    a.request_closed_loop = true;
    a.feed_watchdogs = healthy(i);
    const auto decel = fresh ? config_.decel_units_per_second
                             : config_.timeout_decel_units_per_second;
    a.targets.left_millturns_per_second = left_ramp_.update(0, 5, decel, decel);
    a.targets.right_millturns_per_second =
        right_ramp_.update(0, 5, decel, decel);
    if (stationary(i)) {
      if (stationary_at_ms_ == 0)
        stationary_at_ms_ = i.now_ms;
      if (i.now_ms - stationary_at_ms_ >= config_.standstill_settle_ms) {
        if (fresh && i.command.mode == OperatingMode::Hold)
          enter(MotorState::Holding, i.now_ms);
        else
          enter(MotorState::EnteringManualPush, i.now_ms);
      }
    } else
      stationary_at_ms_ = 0;
    break;
  }
  case MotorState::Holding:
    a.request_closed_loop = true;
    a.feed_watchdogs = true;
    if (fresh && i.command.mode == OperatingMode::ManualPush)
      enter(MotorState::ControlledStopping, i.now_ms);
    else if (fresh && i.command.speed != SpeedLevel::Stopped)
      enter(MotorState::Running, i.now_ms);
    break;
  case MotorState::EnteringManualPush:
    a.request_idle = true;
    if (i.feedback.left.state == AxisState::Idle &&
        i.feedback.right.state == AxisState::Idle)
      enter(MotorState::ManualPush, i.now_ms);
    else if (i.now_ms - entered_at_ms_ > config_.manual_push_timeout_ms)
      latch(FaultCode::TransitionTimeout, i.now_ms);
    break;
  case MotorState::ManualPush:
    a.request_idle = true;
    if (i.recovery_requested && fresh && i.command.speed == SpeedLevel::Stopped)
      enter(MotorState::Recovering, i.now_ms);
    break;
  case MotorState::EmergencyStopped:
    a.request_idle = true;
    if (!i.estop_active && i.recovery_requested && fresh &&
        i.command.speed == SpeedLevel::Stopped)
      enter(MotorState::Recovering, i.now_ms);
    break;
  case MotorState::Fault:
    a.request_idle = true;
    if (i.recovery_requested && fresh &&
        i.command.speed == SpeedLevel::Stopped) {
      enter(MotorState::Recovering, i.now_ms);
      a.clear_errors = true;
    }
    break;
  case MotorState::Recovering:
    a.request_idle = true;
    if (healthy(i) && stationary(i) &&
        i.feedback.left.state == AxisState::Idle &&
        i.feedback.right.state == AxisState::Idle) {
      fault_ = FaultCode::None;
      enter(MotorState::ReadyIdle, i.now_ms);
    } else if (i.now_ms - entered_at_ms_ > config_.transition_timeout_ms)
      latch(FaultCode::TransitionTimeout, i.now_ms);
    break;
  }
  return {state_, fault_, a};
}
} // namespace poor_caddy
