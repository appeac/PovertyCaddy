# Cart state transitions

| State | Entry/action | Guard to leave | Destination |
|---|---|---|---|
| BootSafe | Request both axes idle; zero software ramps | First policy cycle | Initializing |
| Initializing | Establish UART, read both axes and bus, never loop-clear faults | Both axes valid, error-free, idle | ReadyIdle |
| Initializing | Continue safe idle | Transition deadline | Fault |
| ReadyIdle | Both axes idle; no torque | Fresh validated forward command | EnablingClosedLoop |
| EnablingClosedLoop | Zero target, request both axes closed loop | Both axes confirm state 8 | Running |
| EnablingClosedLoop | Never accept one-axis success | Deadline or unhealthy axis | Fault |
| Running | Ramped physical left/right velocity; supervise both axes | Stop or radio lease expiry | ControlledStopping |
| ControlledStopping | Closed-loop ramp to zero | Both measured speeds below threshold for settling interval | Holding or EnteringManualPush |
| Holding | Closed-loop zero target with feedback supervision | Drive request | Running |
| EnteringManualPush | Request both axes idle | Both confirm idle | ManualPush |
| ManualPush | No velocity commands; mechanical brake required on grade | New Stopped command plus deliberate recovery | Recovering |
| EmergencyStopped | Independent hardware action; software requests idle opportunistically | E-stop released, fresh Stopped command, deliberate recovery | Recovering |
| Fault | Preserve first fault; request both idle where possible | Fresh Stopped command and deliberate recovery | Recovering |
| Recovering | One clear-errors action, idle request, full health recheck | Both stationary, healthy, idle | ReadyIdle |

An active e-stop takes precedence from every state. Any required stale feedback, axis error, impossible axis state, overcurrent, or critical bus condition latches the whole cart. Recovery never enters Running directly. A normal radio timeout is not the same as an ODrive watchdog timeout: the former permits a controlled stop while the latter is a fallback for lost host control.

## Wearable intent

Stop alone requests manual-push idle after controlled standstill. Stop plus Left requests hold. Simultaneous Speed Up and Speed Down is the deliberate recovery chord; it always encodes Stopped plus Recover and cannot directly resume motion. A later independent speed-up command is required to drive.
