# Architecture

## Boundaries and data flow

The wearable samples and debounces five active-low buttons at 200 Hz, resolves conservative operator intent (including manual-push, hold, and the stopped recovery chord), and publishes a complete version-2 packet every 50 ms. The receive callback on the cart only bounds and copies bytes. A packet task validates the trusted MAC, explicit wire format, CRC, session, and wrap-safe sequence before overwriting a one-element accepted-command queue.

The cart motor task is the sole owner of `MotorPolicy`. It combines the newest command, monotonic timestamps, the observed e-stop input, and an atomic-style ODrive status snapshot. It emits typed actions. `ODriveUart` is the only task permitted to touch the dedicated ODrive UART; its velocity queue is length one, so the latest generation replaces stale motion intent. It emits both axis commands consecutively and publishes only a complete status snapshot.

Shared protocol, target mapping, ramps, and motor policy contain no ESP-IDF calls and run in host tests. `odrive_protocol` is also host-testable. `odrive_uart` contains the ESP-IDF adapter and firmware-specific scheduling.

## Queue policy

- Raw ESP-NOW: bounded length eight; when full, discard the oldest sample.
- Accepted command: length one, overwrite, latest valid command wins.
- ODrive action: length one, overwrite, generation and age checked again by the UART owner.
- Telemetry: mutex-protected newest complete snapshot; consumers never drain a history of stale readings.

Safety events are recomputed each 5 ms policy cycle rather than queued behind motion. The physical e-stop remains independent of all queues.

## Watchdog layers

ESP-NOW freshness triggers a controlled deceleration while UART and feedback remain healthy. The ODrive communication watchdog handles a different failure: loss or wedging of the ESP32/UART path. It is fed only for healthy closed-loop states with fresh telemetry. The hardware e-stop is a third independent layer and must remove or inhibit torque without firmware cooperation.

ASCII telemetry is intentionally slower than motion updates. Essential telemetry must never be made so verbose that it starves setpoints or watchdog service.
