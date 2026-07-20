# Testing plan

Do not begin with an uncontrolled downhill test.

## Automated

- Build with C++17, `-Wall -Wextra -Werror`.
- Run protocol vectors, malformed/reserved fields, sequence wrap, and session confirmation tests.
- Run control mapping, signs/scales/clamps, contradictory button, and irregular ramp timing tests.
- Run ODrive formatter/parser tests for full-width errors, finite values, partial/overlong/malformed lines, timeout, and response association.
- Use a fake clock/transport to exercise every motor state, each-axis failure, radio/UART timeout, telemetry age, tracking/mismatch persistence, e-stop recovery, bus limits, and first-fault preservation.

## No-power and logic-analyzer

Verify safe boot, dedicated UART voltage and framing, consecutive dual-axis writes, command generation/age rejection, telemetry cadence, watchdog cadence, console separation, e-stop polarity, and no UART motion on boot/recovery.

## Lifted wheels

Start current-limited. Verify motor/encoder direction, low velocity, three speed levels, differential steering, controlled stop, hold current, transition to dual idle, manual rolling resistance, one-axis disconnect, UART disconnect, wearable reset, cart reset, radio blockage, and e-stop from every powered state.

## Mechanical/manual

With power torque-disabled, test positive caster trail, 180-degree swivel, cable stops/loops, shimmy, bumps, asymmetric drivetrain drag, support polygon, parking brake, and loaded push force. Manual push must not be performed with a locked leading caster.

## Progressive energy tests

Using barriers, restraint, instrumentation, an observer, and immediate independent stopping: increase payload, speed, then grade separately. Log bus voltage, regenerative/DC current, phase current, temperature, command/measured velocity, stop distance, BMS behavior, brake resistor temperature, and mechanical-brake performance. Inject radio and UART loss only after normal operation is characterized at that exact energy level.

Final approval requires recorded firmware hash, ODrive configuration backup, GPIOs, credentials provisioned, thresholds, battery/BMS limits, resistor calculations, payload/grade envelope, and reviewer sign-off.
