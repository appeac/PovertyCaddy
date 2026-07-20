# Cart firmware

The ESP32-C6 cart validates ESP-NOW control packets, runs the shared deterministic motor policy at 200 Hz, and supervises both axes of one ODrive v3.6 through a single-owner UART task. Configure GPIOs, credentials, physical velocities, bus/current limits, and timeouts in `main/cart_config.hpp` only after commissioning.

The firmware target is ODrive legacy firmware v0.5.6. Verify the isolated ASCII adapter against the installed firmware before motor power. The e-stop sense pin is not the primary safety circuit.
