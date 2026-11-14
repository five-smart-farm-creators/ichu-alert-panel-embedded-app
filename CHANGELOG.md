# Changelog

All notable changes to this project are documented in this file.

The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/) and the project uses [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [0.1.0] - 2026-11-14

### Added

- Firmware for the local alert panel on an ESP32 DevKitC, which polls the Edge API alert listing every 3 s.
- Panel states Normal, Attention, Urgent and NoConnection, driven by the alerts that are still active.
- 16x2 LCD with the state text and the alert that caused it, three state LEDs and an active buzzer.
- Buzzer alarm for 30 s each time a new Critical alert appears.
- Wokwi simulation with the LCD, the three LEDs and the buzzer.
- Host test of the alert state rules in `tests/`, runnable without the Arduino core.
- `build` job in the Git Flow checks workflow, which compiles the firmware on every pull request.

[Unreleased]: https://github.com/five-smart-farm-creators/ichu-alert-panel-embedded-app/compare/v0.1.0...develop
[0.1.0]: https://github.com/five-smart-farm-creators/ichu-alert-panel-embedded-app/releases/tag/v0.1.0
