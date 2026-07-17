# PoorCaddy

PoorCaddy is an open-source, ESP32-C6 based remote-controlled electric golf push cart. The repository contains two ESP-IDF C++ projects: `wearable/` for the five-button remote and `cart/` for the differential-drive cart controller. Communication is direct encrypted ESP-NOW unicast; there is no AP, cloud service, display, or user-facing LED dependency.

> **Safety notice:** this is a starter implementation for a moving vehicle. Verify every GPIO, polarity, PWM level, and safety interlock on the bench before attaching motor controllers or wheels to the ground.

## Repository layout

```text
poor-caddy/
├── wearable/                       # ESP-IDF wearable remote project
├── cart/                           # ESP-IDF cart controller project
├── components/poor_caddy_protocol/  # Shared protocol and pure logic
├── docs/                           # Architecture, wiring, testing, traceability
├── README.md
└── LICENSE
```

## Hardware assumptions

- ESP32-C6 wearable and ESP32-C6 cart controller.
- Two independent 6-60 V, 400 W class three-phase Hall BLDC motor controllers.
- Motor controller accepts 0-5 V analog or 2.5-5 V PWM at 1-20 kHz. Default firmware uses 10 kHz, 10-bit LEDC PWM, logical range 0-1023.
- ESP32-C6 3.3 V PWM is assumed to exceed the documented 2.5 V minimum, but this **must be verified on the bench**.
- Version 1 does **not** read Hall sensors or the controller speed-pulse output. PWM values are command levels, not verified vehicle speed.
- Direction is forward only. Prefer wiring DIR permanently for forward. The firmware contains no reverse command path.

## Electrical interface warnings

ESP32-C6 GPIOs are 3.3 V only and are not tolerant of arbitrary controller terminal voltages. Do not connect STOP, BRAKE, DIR, or PWM directly until measured. For active-low STOP and BRAKE, design external circuitry that shorts the controller input to controller ground when asserted, such as:

- open-drain N-channel MOSFET,
- open-collector NPN transistor,
- optocoupler when isolation is desired.

Do not power ESP32 boards directly from the cart battery; use an appropriately rated regulated DC-DC supply.

## Build, flash, monitor

Install ESP-IDF for ESP32-C6, then:

```bash
cd poor-caddy/wearable
idf.py set-target esp32c6
idf.py build
idf.py -p /dev/ttyACM0 flash monitor

cd ../cart
idf.py set-target esp32c6
idf.py build
idf.py -p /dev/ttyACM1 flash monitor
```

Host tests for pure logic:

```bash
cd poor-caddy/host_tests
cmake -S . -B build
cmake --build build
./build/poor_caddy_tests
```

## ESP-NOW security provisioning

Both projects include placeholder MACs, PMK, and LMK in editable config headers. Replace all defaults before use. Generate unique 16-byte keys, for example:

```bash
openssl rand -hex 16
```

MAC filtering alone is not authentication. Session IDs are random boot identifiers, not passwords. ESP-NOW peer encryption is the primary protection against unauthorized commands, and repository default keys must never remain in production.

## Documentation

- [Architecture](docs/architecture.md)
- [Cart controller](cart/README.md)
- [Wearable remote](wearable/README.md)
- [Physical testing plan](docs/testing-plan.md)
- [Wiring and interface notes](docs/wiring.md)
- [Cart state transition table](docs/cart-state-transitions.md)
- [Traceability checklist](docs/traceability.md)
- [Troubleshooting](docs/troubleshooting.md)
