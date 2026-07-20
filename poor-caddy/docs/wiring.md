# Wiring and mechanical interface

- Use a dedicated ESP32-C6 UART: ESP TX to ODrive RX, ESP RX to ODrive TX, with verified compatible logic levels and a reference ground or engineered isolation. Do not share the console UART.
- Verify the actual ODrive v3.6 pinout before connection. ESP32-C6 GPIO is not 5 V tolerant.
- The observed e-stop GPIO is diagnostic only. The primary normally-safe e-stop circuit must independently inhibit/remove torque without ESP-NOW, UART, or ESP32 execution. Have its circuit reviewed for the intended hazard category.
- Use a regulated supply for the ESP32. Do not attach battery voltage to logic power.
- Treat battery, BMS, fuse/contactors, precharge, brake resistor, and mechanical brake as one engineered system.
- The swivel module needs positive caster trail. It must be able to reorient about 180 degrees instead of being pushed as a locked leading caster.
- Ordinary motor/encoder wiring must not wind around the kingpin. Use engineered rotation stops and cable loops or a correctly rated slip ring.
- ODrive idle may still have drivetrain drag. A mechanical parking brake is required to hold on a grade.
- Caster trail is directional alignment, not rollover protection. Verify the support polygon, track, wheelbase, center of gravity, load, and slope with the module in every swivel orientation.
