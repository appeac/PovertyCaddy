#include "poor_caddy_protocol/motor_logic.hpp"

#include <algorithm>
#include <cmath>

namespace poor_caddy {
namespace {
float towardZero(float value, float amount) {
    if (value > 0.0F) return std::max(0.0F, value - amount);
    return std::min(0.0F, value + amount);
}
} // namespace

MotorLogic::MotorLogic(MotorConfig config) : config_(config) {}

void MotorLogic::enter(MotorState state, std::uint64_t now_ms) {
    state_ = state;
    state_entered_ms_ = now_ms;
    if (state == MotorState::EmergencyStopped || state == MotorState::Fault)
        recovery_stop_seen_ = false;
}

bool MotorLogic::bothIn(const ODriveStatus& s, AxisState state) const {
    return s.left.state == state && s.right.state == state;
}

bool MotorLogic::stopped(const ODriveStatus& s) const {
    return std::fabs(s.left.velocity) <= config_.standstill_velocity &&
           std::fabs(s.right.velocity) <= config_.standstill_velocity;
}

MotorOutput MotorLogic::output() const {
    MotorOutput result{};
    if (state_ == MotorState::EnablingAxes || state_ == MotorState::Running ||
        state_ == MotorState::ControlledStopping || state_ == MotorState::StationaryHold)
        result.requested_axis_state = AxisState::ClosedLoop;
    result.left_velocity = left_setpoint_;
    result.right_velocity = right_setpoint_;
    return result;
}

MotorOutput MotorLogic::update(std::uint64_t now, const MotorCommand& cmd,
                               const ODriveStatus& status) {
    const std::uint64_t elapsed_ms = last_update_ms_ == 0 ? 0 : now - last_update_ms_;
    last_update_ms_ = now;
    const bool healthy = status.left.healthy && status.right.healthy;

    if (cmd.emergency_stop) {
        left_setpoint_ = right_setpoint_ = 0.0F;
        if (state_ != MotorState::EmergencyStopped) enter(MotorState::EmergencyStopped, now);
        return output();
    }
    if (!healthy && state_ != MotorState::Fault) {
        left_setpoint_ = right_setpoint_ = 0.0F;
        enter(MotorState::Fault, now);
    }

    // A stale Stopped value cannot reset a latched safety event.  It must be a
    // command newly admitted by the session tracker after the event.
    if ((state_ == MotorState::EmergencyStopped || state_ == MotorState::Fault) &&
        cmd.newly_accepted && !cmd.run && !cmd.timed_out)
        recovery_stop_seen_ = true;

    switch (state_) {
    case MotorState::Initializing:
        left_setpoint_ = right_setpoint_ = 0.0F;
        if (!healthy) enter(MotorState::Fault, now);
        else if (bothIn(status, AxisState::Idle)) enter(MotorState::PushableIdle, now);
        else if (now - state_entered_ms_ >= config_.axis_transition_timeout_ms)
            enter(MotorState::Fault, now);
        break;
    case MotorState::PushableIdle:
        if (!bothIn(status, AxisState::Idle)) {
            if (now - state_entered_ms_ >= config_.axis_transition_timeout_ms)
                enter(MotorState::Fault, now);
        } else if (cmd.run && !cmd.timed_out) {
            enter(MotorState::EnablingAxes, now);
        }
        break;
    case MotorState::EnablingAxes:
        if (!cmd.run || cmd.timed_out) enter(MotorState::ControlledStopping, now);
        else if (bothIn(status, AxisState::ClosedLoop)) {
            left_setpoint_ = cmd.left_velocity;
            right_setpoint_ = cmd.right_velocity;
            enter(MotorState::Running, now);
        } else if (now - state_entered_ms_ >= config_.axis_transition_timeout_ms) {
            left_setpoint_ = right_setpoint_ = 0.0F;
            enter(MotorState::Fault, now);
        }
        break;
    case MotorState::Running:
        if (!bothIn(status, AxisState::ClosedLoop)) {
            left_setpoint_ = right_setpoint_ = 0.0F;
            enter(MotorState::Fault, now);
        } else if (!cmd.run || cmd.timed_out) enter(MotorState::ControlledStopping, now);
        else { left_setpoint_ = cmd.left_velocity; right_setpoint_ = cmd.right_velocity; }
        break;
    case MotorState::ControlledStopping: {
        if (!bothIn(status, AxisState::ClosedLoop)) {
            left_setpoint_ = right_setpoint_ = 0.0F;
            enter(MotorState::Fault, now);
            break;
        }
        const float step = config_.stop_deceleration_per_second * elapsed_ms / 1000.0F;
        left_setpoint_ = towardZero(left_setpoint_, step);
        right_setpoint_ = towardZero(right_setpoint_, step);
        if (left_setpoint_ == 0.0F && right_setpoint_ == 0.0F && stopped(status))
            enter(config_.stopped_policy == StoppedPolicy::HoldPosition
                      ? MotorState::StationaryHold : MotorState::PushableIdle, now);
        break;
    }
    case MotorState::StationaryHold:
        left_setpoint_ = right_setpoint_ = 0.0F;
        if (!bothIn(status, AxisState::ClosedLoop)) enter(MotorState::Fault, now);
        else if (cmd.run && !cmd.timed_out) enter(MotorState::Running, now);
        break;
    case MotorState::EmergencyStopped:
    case MotorState::Fault:
        left_setpoint_ = right_setpoint_ = 0.0F;
        if (recovery_stop_seen_ && healthy) enter(MotorState::Recovering, now);
        break;
    case MotorState::Recovering:
        if (!healthy) enter(MotorState::Fault, now);
        else if (bothIn(status, AxisState::Idle)) enter(MotorState::PushableIdle, now);
        else if (now - state_entered_ms_ >= config_.axis_transition_timeout_ms)
            enter(MotorState::Fault, now);
        break;
    }
    return output();
}

} // namespace poor_caddy
