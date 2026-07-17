#include "poor_caddy_protocol/ramp.hpp"
#include <algorithm>
namespace poor_caddy {
std::uint16_t Ramp::update(std::uint16_t target, std::uint32_t elapsed_ms) {
    const std::uint32_t clamped = std::min(elapsed_ms, 20U);
    if (value_ == target) { rem_ = 0; return value_; }
    const std::uint64_t scaled = static_cast<std::uint64_t>(rate_) * clamped + rem_;
    const std::uint32_t step = static_cast<std::uint32_t>(scaled / 1000U);
    rem_ = static_cast<std::uint32_t>(scaled % 1000U);
    if (step == 0U) return value_;
    if (target > value_) {
        value_ = static_cast<std::uint16_t>(std::min<std::uint32_t>(target, static_cast<std::uint32_t>(value_) + step));
    } else {
        const std::uint32_t delta = static_cast<std::uint32_t>(value_ - target);
        value_ = static_cast<std::uint16_t>((step >= delta) ? target : static_cast<std::uint16_t>(value_ - step));
    }
    if (value_ == target) rem_ = 0;
    return value_;
}
}
