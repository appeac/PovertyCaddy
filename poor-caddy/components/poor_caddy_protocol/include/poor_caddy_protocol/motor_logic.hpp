#pragma once
#include <cstdint>
#include "poor_caddy_protocol/control.hpp"
namespace poor_caddy { enum class MotorState: std::uint8_t { PushableIdle, BrakeReleasing, Running, Coasting, Braked, EmergencyStopped, Fault }; struct MotorConfig { std::uint32_t link_timeout_ms{1000}; std::uint32_t coast_delay_ms{2500}; std::uint32_t brake_release_ms{150}; }; }
