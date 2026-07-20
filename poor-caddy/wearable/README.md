# Wearable firmware

The ESP32-C6 wearable debounces five active-low buttons, resolves Stop conservatively, and sends a full encrypted version-2 ESP-NOW packet every 50 ms with a random boot session and advancing sequence.

Replace GPIO/MAC/PMK/LMK placeholders in `main/wearable_config.hpp`. Powered reverse is deliberately unavailable.

## Stop-mode gestures

- Stop alone requests a controlled stop followed by manual-push idle.
- Stop plus Left requests a controlled stop followed by zero-velocity hold.
- A simultaneous Speed Up and Speed Down press requests recovery while remaining stopped; this is the deliberate recovery chord and never directly commands motion.
