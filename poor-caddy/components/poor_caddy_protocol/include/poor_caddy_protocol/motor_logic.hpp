#pragma once

#include <cstdint>

namespace poor_caddy {

enum class MotorState : std::uint8_t {
    Initializing,
    EnablingAxes,
    Running,
    ControlledStopping,
    StationaryHold,
    PushableIdle,
    EmergencyStopped,
    Fault,
    Recovering,
};

enum class AxisState : std::uint8_t { Unknown, Idle, ClosedLoop };
enum class StoppedPolicy : std::uint8_t { HoldPosition, Pushable };

struct AxisStatus {
    AxisState state{AxisState::Unknown};
    bool healthy{true};
    float velocity{0.0F};
};

struct ODriveStatus {
    AxisStatus left{};
    AxisStatus right{};
};

// Input is deliberately independent of the radio protocol and ODrive transport.
struct MotorCommand {
    bool run{false};
    bool emergency_stop{false};
    bool timed_out{false};
    // Set for one update only when a command passed session/sequence validation.
    bool newly_accepted{false};
    float left_velocity{0.0F};
    float right_velocity{0.0F};
};

struct MotorConfig {
    std::uint32_t link_timeout_ms{1000};
    std::uint32_t axis_transition_timeout_ms{1000};
    float stop_deceleration_per_second{400.0F};
    float standstill_velocity{5.0F};
    StoppedPolicy stopped_policy{StoppedPolicy::Pushable};
};

struct MotorOutput {
    AxisState requested_axis_state{AxisState::Idle};
    float left_velocity{0.0F};
    float right_velocity{0.0F};
};

class MotorLogic {
public:
    explicit MotorLogic(MotorConfig config = {});
    MotorOutput update(std::uint64_t now_ms, const MotorCommand& command,
                       const ODriveStatus& status);
    MotorState state() const { return state_; }

private:
    void enter(MotorState state, std::uint64_t now_ms);
    MotorOutput output() const;
    bool bothIn(const ODriveStatus& status, AxisState state) const;
    bool stopped(const ODriveStatus& status) const;

    MotorConfig config_;
    MotorState state_{MotorState::Initializing};
    std::uint64_t state_entered_ms_{0};
    std::uint64_t last_update_ms_{0};
    float left_setpoint_{0.0F};
    float right_setpoint_{0.0F};
    bool recovery_stop_seen_{false};
};

} // namespace poor_caddy
