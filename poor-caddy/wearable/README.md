# PoorCaddy wearable

The wearable owns the desired `ControlState` and transmits the complete current state at 20 Hz. Five active-low buttons use ESP32-C6 internal pull-ups and are sampled every 5 ms with about 25 ms debounce.

| Function | Placeholder GPIO | Notes |
|---|---:|---|
| Speed Up | 2 | Verify against board flash/USB/JTAG/bootstrap pins |
| Speed Down | 3 | Verify before wiring |
| Left | 4 | Held steering input |
| Right | 5 | Held steering input |
| Stop | 6 | Held stop input, priority over all controls |

Edit `main/wearable_config.hpp` before connecting hardware. Speed buttons are one-shot press events. Stop and steering are held states. Speed Up + Speed Down latches Stopped. Left + Right resolves to Straight.
