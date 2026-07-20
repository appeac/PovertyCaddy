#pragma once
#include "poor_caddy_protocol/control.hpp"
#include <cstdint>
namespace poor_caddy {
class VelocityRamp {
public:
  void reset(VelocityMilliTurnsPerSecond v = 0) {
    value_ = v;
    remainder_ = 0;
  }
  VelocityMilliTurnsPerSecond update(VelocityMilliTurnsPerSecond target,
                                     std::uint32_t elapsed_ms,
                                     std::uint32_t accel_units_per_second,
                                     std::uint32_t decel_units_per_second);
  VelocityMilliTurnsPerSecond value() const { return value_; }

private:
  VelocityMilliTurnsPerSecond value_{};
  std::uint32_t remainder_{};
};
} // namespace poor_caddy
