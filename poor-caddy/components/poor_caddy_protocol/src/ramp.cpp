#include "poor_caddy_protocol/ramp.hpp"
#include <algorithm>
#include <cstdint>
namespace poor_caddy {
VelocityMilliTurnsPerSecond
VelocityRamp::update(VelocityMilliTurnsPerSecond target, std::uint32_t ms,
                     std::uint32_t accel, std::uint32_t decel) {
  ms = std::min(ms, 50U);
  const bool slowing = (value_ > 0 && target < value_) ||
                       (value_ < 0 && target > value_) || target == 0;
  const auto rate = slowing ? decel : accel;
  const std::uint64_t scaled =
      static_cast<std::uint64_t>(rate) * ms + remainder_;
  const std::int64_t step = static_cast<std::int64_t>(scaled / 1000U);
  remainder_ = static_cast<std::uint32_t>(scaled % 1000U);
  if (target > value_)
    value_ = static_cast<VelocityMilliTurnsPerSecond>(std::min<std::int64_t>(
        target, static_cast<std::int64_t>(value_) + step));
  else if (target < value_)
    value_ = static_cast<VelocityMilliTurnsPerSecond>(std::max<std::int64_t>(
        target, static_cast<std::int64_t>(value_) - step));
  else
    remainder_ = 0;
  if (value_ == target)
    remainder_ = 0;
  return value_;
}
} // namespace poor_caddy
