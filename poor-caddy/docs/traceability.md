# Traceability checklist

| Requirement area | Implementation | Tests/docs |
|---|---|---|
| Two ESP-IDF projects and shared component | `wearable/`, `cart/`, `components/poor_caddy_protocol/` | root README |
| Explicit speed/steering enums | `components/poor_caddy_protocol/include/poor_caddy_protocol/control.hpp` | host test |
| Button debounce and resolution | `wearable/main/main.cpp`, `control_logic.cpp` | Stage 1 |
| One-item wearable state queue and 20 Hz transmit | `wearable/main/main.cpp` | architecture doc |
| Explicit packet serialization, CRC-32 | `protocol.cpp`, `crc32.cpp` | `test_protocol.cpp` |
| ESP-NOW encryption placeholders | `wearable_config.hpp`, `cart_config.hpp` | README security section |
| Random session and sequence | `wearable/main/main.cpp` | architecture doc |
| Lightweight cart callback and raw queue overflow policy | `cart/main/main.cpp` | architecture doc |
| Validation order/session/sequence | `cart/main/main.cpp`, `session_tracker.cpp`, `sequence.cpp` | host test |
| Three-packet startup confirmation | `session_tracker.cpp` | host test |
| Accepted-command overwrite queue | `cart/main/main.cpp` | architecture doc |
| 1000 ms timeout and coasting/braking | `cart/main/main.cpp` | state table, Stage 1 |
| 200 Hz motor task with elapsed ramp clamp | `cart/main/main.cpp`, `ramp.cpp` | host test |
| E-stop policy and recovery gate | `cart/main/main.cpp`, cart README | Stage 1/5/8 |
| Motor states | `motor_logic.hpp`, `cart/main/main.cpp` | state transition doc |
| Three speed levels and steering reductions | `control_logic.cpp` | host test |
| PWM/output abstraction | `cart/main/main.cpp` `MotorHw` | Stage 2 |
| Electrical warnings/no Hall feedback | README, `docs/wiring.md` | testing plan |
| Diagnostics counters | `wearable/main/main.cpp`, `cart/main/main.cpp` | serial monitor during tests |
| Monotonic time | `nowMs()` helpers | architecture doc |
| Error handling safe outputs | `cart/main/main.cpp` | troubleshooting |
