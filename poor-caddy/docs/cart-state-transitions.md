# Cart state transition table

| State | Outputs | Exit condition | Next state |
|---|---|---|---|
| PushableIdle | PWM zero, STOP asserted as configured, BRAKE released | Three valid advancing packets accepted and latest command is forward | BrakeReleasing |
| BrakeReleasing | PWM zero, STOP released, BRAKE released | 150 ms elapsed and command still forward | Running |
| BrakeReleasing | PWM zero | stop, timeout, fault, or e-stop | Coasting, EmergencyStopped, or Fault |
| Running | BRAKE/STOP released, ramped left/right PWM | Stop command or timeout | Coasting |
| Running | PWM zero, STOP/BRAKE asserted | e-stop | EmergencyStopped |
| Coasting | PWM zero, BRAKE released, STOP configurable | 2500 ms elapsed | Braked |
| Coasting | PWM zero, BRAKE released | newer forward command before brake applies | BrakeReleasing |
| Braked | PWM zero, STOP asserted, BRAKE asserted | valid forward command | BrakeReleasing |
| EmergencyStopped | PWM zero, STOP asserted, BRAKE asserted | e-stop released and a new valid Stopped command received | Braked |
| Fault | PWM zero, STOP asserted, BRAKE asserted | explicit reset/defined recovery | PushableIdle or Braked |
