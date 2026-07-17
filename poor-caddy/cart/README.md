# PoorCaddy cart controller

The cart owns safety enforcement, packet validation, communication timeout, motor state transitions, brake timing, PWM ramping, e-stop handling, and physical outputs.

| Function | Placeholder GPIO | Notes |
|---|---:|---|
| Left PWM | 2 | LEDC 10 kHz, 10-bit default |
| Right PWM | 3 | LEDC 10 kHz, 10-bit default |
| Left STOP | 4 | Semantic asserted output; external open-drain/isolated interface recommended |
| Right STOP | 5 | Same as above |
| Left BRAKE | 6 | Semantic asserted output; external interface recommended |
| Right BRAKE | 7 | Same as above |
| E-stop aux | 8 | Default active-low with pull-up |

GPIOs are placeholders. Validate against flash pins, USB/JTAG, boot straps, onboard RGB LED, and unavailable board pins. Physical e-stop must hardwire interruption of the motor-control circuit independently of the ESP32; the GPIO is only an auxiliary software input.

Default behavior: boot in `PushableIdle` with PWM zero and brakes released; require three valid advancing packets from one session before accepting motion; timeout after 1000 ms of no valid active-session packet; soft stop coasts 2500 ms then brakes; e-stop immediately zeros PWM, asserts STOP, applies BRAKE, and requires release plus a newly received Stopped command before future motion.
