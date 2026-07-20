# Requirement traceability

| Requirement | Implementation | Verification |
|---|---|---|
| Explicit versioned radio packet | `protocol.hpp/.cpp` | `test_protocol.cpp` |
| Startup/session confirmation | `session_tracker.cpp` | `test_protocol.cpp` |
| Physical wheel velocities | `control.hpp`, `control_logic.cpp` | `test_control.cpp` |
| Deterministic asymmetric ramp | `ramp.hpp/.cpp` | `test_control.cpp` |
| Pure safety state machine | `motor_policy.hpp/.cpp` | `test_motor_policy.cpp` |
| Single UART owner | `odrive_uart.hpp/.cpp` | review and logic analyzer |
| Firmware-isolated ASCII | `odrive_protocol.hpp/.cpp` | `test_odrive_protocol.cpp` and bench verification |
| Independent e-stop | external hardware plus sense GPIO | electrical review and fault injection |
| Regeneration envelope | configuration and commissioning record | progressively loaded grade tests |
| Manual push after confirmed idle | motor policy and mechanical design | state test and manual test |
