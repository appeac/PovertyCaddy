#pragma once
#include <cstdint>
namespace poor_caddy { enum class SequenceClass : std::uint8_t { First, Newer, Duplicate, Old }; SequenceClass classifySequence(bool have_last, std::uint32_t last, std::uint32_t candidate); std::uint32_t missingBetween(std::uint32_t last, std::uint32_t candidate); }
