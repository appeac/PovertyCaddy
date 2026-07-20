#pragma once
#include "poor_caddy_protocol/control.hpp"
#include "poor_caddy_protocol/ramp.hpp"
#include <cstdint>
namespace poor_caddy {
enum class MotorState : std::uint8_t {
  BootSafe,
  Initializing,
  ReadyIdle,
  EnablingClosedLoop,
  Running,
  ControlledStopping,
  Holding,
  EnteringManualPush,
  ManualPush,
  EmergencyStopped,
  Fault,
  Recovering
};
enum class FaultCode : std::uint8_t {
  None,
  Estop,
  Uart,
  TelemetryStale,
  AxisError,
  AxisState,
  BusVoltage,
  OverCurrent,
  TrackingError,
  TransitionTimeout
};
enum class AxisState : std::uint8_t { Undefined = 0, Idle = 1, ClosedLoop = 8 };
struct AxisFeedback {
  bool valid{};
  AxisState state{AxisState::Undefined};
  std::uint64_t errors{};
  VelocityMilliTurnsPerSecond velocity{};
  std::int32_t current_milliamps{};
  std::uint64_t updated_at_ms{};
};
struct MotorFeedback {
  bool uart_healthy{};
  AxisFeedback left{}, right{};
  std::int32_t bus_millivolts{};
  std::uint64_t bus_updated_at_ms{};
};
struct MotorPolicyConfig {
  DriveConfig drive{};
  std::uint32_t link_timeout_ms{750}, telemetry_max_age_ms{300},
      transition_timeout_ms{2000}, standstill_settle_ms{300},
      manual_push_timeout_ms{2000};
  VelocityMilliTurnsPerSecond standstill_velocity{100}, tracking_error{1000};
  std::int32_t max_current_milliamps{30000}, min_bus_millivolts{10000},
      critical_bus_millivolts{56000};
  std::uint32_t accel_units_per_second{1000}, decel_units_per_second{2000},
      timeout_decel_units_per_second{3000};
};
struct PolicyInput {
  std::uint64_t now_ms{};
  bool estop_active{};
  bool have_command{};
  bool recovery_requested{};
  ControlState command{};
  std::uint64_t command_at_ms{};
  MotorFeedback feedback{};
};
struct MotorAction {
  WheelVelocityTargets targets{};
  bool request_idle{};
  bool request_closed_loop{};
  bool clear_errors{};
  bool feed_watchdogs{};
};
struct PolicyOutput {
  MotorState state{MotorState::BootSafe};
  FaultCode fault{FaultCode::None};
  MotorAction action{};
};
class MotorPolicy {
public:
  explicit MotorPolicy(MotorPolicyConfig config = {}) : config_(config) {}
  PolicyOutput update(const PolicyInput &);
  MotorState state() const { return state_; }
  FaultCode fault() const { return fault_; }

private:
  bool healthy(const PolicyInput &) const;
  bool stationary(const PolicyInput &) const;
  void enter(MotorState, std::uint64_t);
  void latch(FaultCode, std::uint64_t);
  MotorPolicyConfig config_;
  MotorState state_{MotorState::BootSafe};
  FaultCode fault_{FaultCode::None};
  std::uint64_t entered_at_ms_{}, stationary_at_ms_{};
  VelocityRamp left_ramp_, right_ramp_;
};
} // namespace poor_caddy
