#pragma once
#include <array>
#include <cstdint>
namespace cart_config { constexpr int kLeftPwmGpio=2,kRightPwmGpio=3,kLeftStopGpio=4,kRightStopGpio=5,kLeftBrakeGpio=6,kRightBrakeGpio=7,kEstopGpio=8; constexpr bool kDigitalAssertedLevelLow=true,kEstopActiveLow=true,kAssertStopDuringCoast=false; constexpr std::uint32_t kPwmFrequencyHz=10000; constexpr std::uint16_t kPwmMaxDuty=1023; constexpr std::array<std::uint8_t,6> kWearableMac{{0x24,0x6F,0x28,0,0,1}}; constexpr std::array<std::uint8_t,6> kCartMac{{0x24,0x6F,0x28,0,0,2}}; constexpr std::array<std::uint8_t,16> kPmk{{'R','E','P','L','A','C','E','_','P','M','K','_','1','6','B','!'}}; constexpr std::array<std::uint8_t,16> kLmk{{'R','E','P','L','A','C','E','_','L','M','K','_','1','6','B','!'}}; }
