#pragma once
#include <cstdint>
namespace poor_caddy { class Ramp { public: explicit Ramp(std::uint32_t units_per_second=250): rate_(units_per_second){} void reset(std::uint16_t v){value_=v; rem_=0;} std::uint16_t update(std::uint16_t target, std::uint32_t elapsed_ms); std::uint16_t value() const {return value_;} private: std::uint16_t value_{}; std::uint32_t rate_{}; std::uint32_t rem_{}; }; }
