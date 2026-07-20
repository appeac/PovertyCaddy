# PovertyCaddy ODrive firmware

PovertyCaddy is an ESP-IDF C++17 reference implementation for an ESP32-C6 wearable and cart controller. The wearable sends authenticated, sequenced ESP-NOW commands. The cart validates them, runs a deterministic safety policy, and gives physical wheel-velocity commands to both axes of one ODrive v3.6 over a dedicated UART.

This repository is not a safety certification. Before powered testing, replace credentials and GPIO placeholders and validate the independent e-stop, battery/BMS regeneration behavior, brake resistor, mechanical brake, motor/encoder setup, caster geometry, cable routing, payload, and grade envelope.

Build host tests with:

```sh
cmake -S host_tests -B build/host
cmake --build build/host
ctest --test-dir build/host --output-on-failure
```

See `docs/architecture.md`, `docs/odrive-commissioning.md`, and `docs/testing-plan.md` before connecting motor power.
