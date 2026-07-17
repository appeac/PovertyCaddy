#pragma once
#include <array>
#include <cstdint>
namespace wearable_config {
constexpr int kButtonSpeedUpGpio = 2; constexpr int kButtonSpeedDownGpio = 3; constexpr int kButtonLeftGpio = 4; constexpr int kButtonRightGpio = 5; constexpr int kButtonStopGpio = 6; // PLACEHOLDERS: verify on your ESP32-C6 board.
constexpr std::array<std::uint8_t,6> kCartMac{{0x24,0x6F,0x28,0x00,0x00,0x02}}; constexpr std::array<std::uint8_t,6> kWearableMac{{0x24,0x6F,0x28,0x00,0x00,0x01}};
constexpr std::array<std::uint8_t,16> kPmk{{'R','E','P','L','A','C','E','_','P','M','K','_','1','6','B','!'}}; constexpr std::array<std::uint8_t,16> kLmk{{'R','E','P','L','A','C','E','_','L','M','K','_','1','6','B','!'}};
}
