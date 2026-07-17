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
