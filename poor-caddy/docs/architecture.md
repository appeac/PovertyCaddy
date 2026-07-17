# Architecture

PoorCaddy separates hardware configuration, radio transport, packet protocol, validation/session logic, motor state control, ramping, and GPIO/PWM output.

## Wearable

- `main/wearable_config.hpp`: placeholder GPIOs, MACs, PMK, LMK.
- Button task: 5 ms `vTaskDelayUntil()`, debounced buttons, `resolveButtons()`, one-item queue, `xQueueOverwrite()`.
- Transmit task: 50 ms `vTaskDelayUntil()`, random 32-bit session ID from `esp_random()`, sequence starts at zero and increments per send attempt, sends full packet every period.

## Shared component

`components/poor_caddy_protocol` is ESP-IDF independent where practical. It includes explicit `SpeedLevel` and `SteeringState` enums, fixed byte protocol encoding, CRC-32, sequence classification with half-range modular assumption, session/startup confirmation, drive target selection, and deterministic fixed-remainder ramps.

Wire layout is little-endian explicit bytes: magic at 0, session at 4, sequence at 8, version/speed/steering/flags at 12-15, reserved zero bytes at 16-19, CRC-32 at 20-23 over bytes 0-19.

## Cart

- ESP-NOW receive callback copies sender MAC and bounded bytes to an ordinary length-8 queue without blocking.
- Packet task validates sender, length, magic, version, CRC, enum ranges, session, and sequence before publishing `AcceptedCommand` to a one-item overwrite queue.
- Motor task runs at 200 Hz, checks e-stop and timeout each cycle, updates state machine and ramps using measured elapsed milliseconds, and writes outputs through `MotorHw`.
