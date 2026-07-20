# Troubleshooting

Always restrain/lift wheels before diagnosing a motion fault. Preserve the first latched fault.

- **Will not enter closed loop:** inspect both axis states/errors, calibration, encoder direction, motor configuration, bus voltage, and transition deadline. Do not repeatedly clear errors.
- **One axis fails:** the whole cart must fault. Check that side's power, encoder, motor, and UART property responses; never continue one-wheel drive.
- **UART timeout/parse errors:** verify dedicated port, crossover, ground/level compatibility, baud, line ending, firmware v0.5.6 property names, response length, and that no console task consumes bytes.
- **Watchdog trips:** confirm persistent configuration, feed grammar, task cadence, telemetry workload, and that policy intentionally withholds feeds when unhealthy.
- **Encoder sign mismatch:** correct axis mapping/direction under lifted-wheel conditions; never compensate blindly if measured velocity has the opposite sign.
- **Tracking or wheel mismatch:** inspect load, tire pressure, caster alignment, encoder calibration, current limit, binding, and drivetrain drag before widening thresholds.
- **Bus overvoltage:** stop grade testing. Review BMS charge acceptance/disconnect, battery state of charge, DC limits, resistor sizing/configuration, and mechanical braking.
- **Manual mode resists pushing:** confirm both axes report idle; then measure gearbox, bearing, motor cogging, and asymmetric drag.
- **Caster shimmy/cable interference:** reduce test speed, inspect trail/kingpin play/wheel balance, and redesign rotation stops or routing. Do not use cable tension as a steering stop.
