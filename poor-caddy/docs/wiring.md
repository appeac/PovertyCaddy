# Wiring and interface notes

- Verify controller terminal voltages before connection. ESP32-C6 pins are not 5 V tolerant and are not tolerant of motor-controller pullups above 3.3 V.
- Use a shared control ground only after bench verification. Use optocouplers where isolation is desired.
- STOP and BRAKE are active by shorting to ground on the referenced controller; firmware models outputs semantically as `asserted`, with inversion configurable, so external MOSFET/NPN/opto circuits can be used.
- DIR should be permanently wired/configured for forward. Firmware does not energize or toggle reverse in normal operation.
- Do not use an ohmmeter or continuity mode on an energized circuit.
- Use a regulated DC-DC supply for ESP32 power; never connect raw cart battery voltage to ESP32 power pins.
