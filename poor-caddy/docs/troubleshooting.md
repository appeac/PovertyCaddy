# Troubleshooting

- No packets accepted: verify MAC addresses, PMK/LMK, ESP-NOW peer encryption, and both boards on compatible channels.
- Cart never moves after reset: three valid advancing packets from the same session are required before motion.
- Cart times out while moving: inspect antenna placement and body blockage; only fully validated newer active-session packets refresh the 1000 ms timeout.
- Motors spin backward: fix motor phase/Hall wiring or permanent DIR wiring; do not add software reverse.
- STOP/BRAKE inverted: adjust output inversion after measuring the external interface with motor controllers disconnected.
- PWM ignored: test 1 kHz, 5 kHz, and 10 kHz and verify 3.3 V PWM amplitude is recognized by the controller.
