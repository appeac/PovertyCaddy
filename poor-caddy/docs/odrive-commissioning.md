# ODrive v3.6 commissioning

## Pinned compatibility target

The adapter is pinned to **ODrive legacy firmware v0.5.6 on ODrive v3.6 hardware**. The command grammar is isolated in `cart/main/odrive_protocol.cpp`. The intended legacy ASCII forms are `v <axis> <velocity> <torque_ff>`, `w <property> <value>`, `r <property>`, `u <axis>`, and `sc`. Before motor power, compare every form and property path against the v0.5.6 source/tag and test it over a current-limited bench connection. Network access to authoritative ODrive material was unavailable during this rebuild, so this verification is a release gate, not an optional check.

Expected properties are `axisN.current_state`, `axisN.encoder.vel_estimate`, error fields on axis/motor/encoder/controller, `axisN.motor.current_control.Iq_measured`, and `vbus_voltage`. Never substitute commands from a newer Fibre-based product without updating and retesting the adapter.

## Persistent configuration record

Record and independently review before enabling wheels:

| Item | Axis 0 | Axis 1 |
|---|---:|---:|
| Physical side | left | right |
| Encoder CPR/type | | |
| Motor pole pairs/type | | |
| Direction sign | +1 default | -1 default |
| Velocity limit (turn/s) | | |
| Motor current limit (A) | | |
| DC current limits (A) | | |
| Watchdog enabled/timeout/action | | |

Also record firmware hash, UART baud/pin mapping, control mode, input mode, calibration results, thermistor behavior, brake resistor resistance/power/enabling, battery maximum voltage and charge current, and BMS response to regeneration/disconnection.

## Guarded sequence

1. Mechanically restrain the cart with both drive wheels lifted and an independent e-stop accessible.
2. Verify phase, encoder, and direction configuration one axis at a time using official ODrive tooling.
3. Configure conservative velocity/current limits and watchdog persistently.
4. With motor power limited, prove `r`, state requests, watchdog feed, velocity zero, low positive velocity, stop, and idle on each axis.
5. Prove the ESP32 treats either-axis loss as a whole-cart fault.
6. Only then commission the integrated dual-axis state machine.

There is no unrestricted ASCII console passthrough. Commissioning must go through typed bounded actions. Software passing tests does not validate the energy path on a hill.

## Regeneration release gate

Document battery maximum voltage and charge current, whether its BMS can disconnect during regeneration, brake resistor calculations/configuration, DC current limits, motor/controller thermal limits, maximum payload/speed/grade, and an independent mechanical brake. A bus overvoltage trip can remove active braking; progressively loaded restrained and graded testing is mandatory.
