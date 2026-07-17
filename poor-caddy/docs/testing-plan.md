# Physical testing plan

## Stage 1: software-only tests

- [ ] Build both projects.
- [ ] Run protocol unit tests.
- [ ] Test CRC known vectors.
- [ ] Test sequence wraparound.
- [ ] Test old and duplicate packet rejection.
- [ ] Test session replacement.
- [ ] Test startup three-packet confirmation.
- [ ] Test timeout state transitions.
- [ ] Test all button combinations.
- [ ] Test e-stop recovery rules.
- [ ] Test ramp timing with simulated elapsed times.

## Stage 2: GPIO tests with no motor controllers

- [ ] Use LEDs, logic analyzer, or oscilloscope.
- [ ] Verify all outputs begin in safe states.
- [ ] Verify left/right PWM frequencies.
- [ ] Verify 0%, intermediate, and maximum duties.
- [ ] Verify STOP polarity.
- [ ] Verify BRAKE polarity.
- [ ] Verify e-stop polarity.
- [ ] Verify no reverse output changes.

## Stage 3: characterize one unpowered controller

With the motor controller disconnected from high-power supply:

- [ ] Inspect continuity and input circuitry where practical.
- [ ] Identify controller signal ground.
- [ ] Verify whether STOP/BRAKE are internally pulled up.
- [ ] Measure expected open-circuit voltages safely.
- [ ] Determine whether direct 3.3 V logic is safe.
- [ ] Prefer transistor or optocoupler interfacing if uncertain.

Never use an ohmmeter or continuity mode on an energized circuit.

## Stage 4: one controller, low-risk powered bench test

- [ ] Use a current-limited supply if available.
- [ ] Lift the motor wheel from the ground.
- [ ] Confirm Hall and phase wiring.
- [ ] Confirm PWM zero does not start the motor.
- [ ] Confirm STOP operation.
- [ ] Confirm BRAKE operation.
- [ ] Confirm 3.3 V PWM is recognized.
- [ ] Test 1 kHz, 5 kHz, and 10 kHz if needed.
- [ ] Select a stable frequency within 1-20 kHz.
- [ ] Start with very low duty.
- [ ] Record minimum reliable starting duty.
- [ ] Observe controller and motor temperature.
- [ ] Never exceed controller or motor ratings.

## Stage 5: dual-controller lifted-wheel test

- [ ] Both powered wheels off the ground.
- [ ] Confirm both rotate only forward.
- [ ] Confirm equal straight commands.
- [ ] Confirm left steering slows only left wheel.
- [ ] Confirm right steering slows only right wheel.
- [ ] Confirm Stop sets both PWM outputs to zero.
- [ ] Confirm coast delay.
- [ ] Confirm both brakes.
- [ ] Confirm communication timeout.
- [ ] Confirm wearable reboot behavior.
- [ ] Confirm cart reboot behavior.
- [ ] Confirm e-stop action.

## Stage 6: supported-load test

- [ ] Use blocks or a stand.
- [ ] Apply realistic wheel load without allowing travel.
- [ ] Tune Forward1, Forward2, Forward3.
- [ ] Tune steering reductions.
- [ ] Verify controller current and heat.
- [ ] Verify no unintended startup motion.

## Stage 7: low-speed ground test

- [ ] Open area, no people or obstacles nearby.
- [ ] Immediate access to physical e-stop.
- [ ] Begin with Forward1 values reduced well below defaults.
- [ ] Verify straight tracking, steering, radio blockage behavior, stop distance.
- [ ] Tune coast and brake delays.

## Stage 8: fault-injection test

- [ ] Power off wearable while moving slowly.
- [ ] Block radio temporarily.
- [ ] Send duplicate, old, and bad CRC packets.
- [ ] Reboot wearable and cart.
- [ ] Activate e-stop.
- [ ] Disconnect one controller signal.
- [ ] Confirm safe behavior in every case.

## Final record

| Item | Final value |
|---|---|
| GPIO assignments | |
| Output inversion | |
| PWM frequency | |
| PWM resolution | |
| Minimum starting duty | |
| Forward1/2/3 values | |
| Steering reductions | |
| Coast delay | |
| Brake-release delay | |
| E-stop polarity | |
| PMK/LMK provisioning status | |

## Guarded cart calibration mode

Calibration mode is a bench-only firmware mode for characterizing controller PWM duty and wheel speed feedback from a host computer over the ESP32 USB serial console. It must **not** be enabled for normal driving: when `kCalibrationModeEnabled` is `true`, the cart intentionally skips ESP-NOW initialization and does not run the normal packet-processing task.

To enable it, set `kCalibrationModeEnabled` in `cart/main/cart_config.hpp` to `true`, confirm `kCalibrationMaxDuty` is low enough for the bench setup, and rebuild/flash the cart firmware. Leave `kLeftSpeedPulseGpio` and `kRightSpeedPulseGpio` at `-1` when no speed pulse sensors are connected; otherwise set them to input-only GPIOs connected to conditioned speed pulse signals. Calibration firmware boots with PWM at zero, STOP asserted, and BRAKE applied so the operator must deliberately release BRAKE before motion testing.

Serial commands are newline terminated and machine oriented:

- `PWM <duty>` sets both channels to the clamped duty.
- `LEFT <duty>` and `RIGHT <duty>` set one channel.
- `STOP` sets both PWM channels to zero and asserts STOP.
- `BRAKE 0` releases brake; `BRAKE 1` applies brake and zeros PWM.
- `STATUS` prints the current state.

Every duty is clamped to `0..kCalibrationMaxDuty`. Nonzero PWM commands are refused while e-stop is active. Malformed motion/brake commands use a conservative policy: the firmware zeros PWM, asserts STOP, prints `ERR`, and then prints status. Malformed non-motion commands leave outputs unchanged and print `ERR` plus status.

Expected low-rate feedback looks like:

```text
CAL duty_left=120 duty_right=120 estop=0 brake=0 stop=0 left_pulses=15 right_pulses=14 window_ms=250
```

### Host Python calibration workflow

Install `pyserial`, connect the ESP32 over USB, and replace `/dev/ttyUSB0` with the cart serial port (`COMx` on Windows). The script sends `BRAKE 0` and `PWM <duty>`, reads `CAL ...` lines, and converts pulse counts for each report window to RPM and MPH. Set `PULSES_PER_REV`, `WHEEL_DIAMETER_IN`, and the test duty for the actual hardware before running.

```python
import math
import serial

PORT = "/dev/ttyUSB0"
BAUD = 115200
PULSES_PER_REV = 20
WHEEL_DIAMETER_IN = 10.0
DUTY = 120

last_left = 0
last_right = 0

with serial.Serial(PORT, BAUD, timeout=2) as ser:
    ser.write(b"BRAKE 0\n")
    ser.write(f"PWM {DUTY}\n".encode())
    for raw in ser:
        line = raw.decode(errors="replace").strip()
        if not line.startswith("CAL "):
            continue
        fields = dict(item.split("=", 1) for item in line.split()[1:])
        window_ms = int(fields["window_ms"])
        if window_ms <= 0:
            continue
        left = int(fields["left_pulses"])
        right = int(fields["right_pulses"])
        delta_left = left - last_left
        delta_right = right - last_right
        last_left, last_right = left, right

        def to_rpm_mph(pulses):
            revs = pulses / PULSES_PER_REV
            minutes = window_ms / 60000.0
            rpm = revs / minutes
            mph = rpm * math.pi * WHEEL_DIAMETER_IN * 60.0 / 63360.0
            return rpm, mph

        left_rpm, left_mph = to_rpm_mph(delta_left)
        right_rpm, right_mph = to_rpm_mph(delta_right)
        print(f"left={left_rpm:.1f} rpm {left_mph:.2f} mph; "
              f"right={right_rpm:.1f} rpm {right_mph:.2f} mph")
```

During calibration, keep driven wheels off the ground, use current limiting where practical, keep immediate access to the physical e-stop, and return `kCalibrationModeEnabled` to `false` before any wearable-controlled or ground-driving test.
