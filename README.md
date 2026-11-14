# ICHU Alert Panel Embedded Application

Firmware of the ICHU alert panel, the local display that tells the ranch staff whether the herd needs attention. It runs on an ESP32 DevKitC, reads the alert listing of the Edge API every 3 seconds and shows the result on a 16x2 LCD, three LEDs and a buzzer.

---

## Overview

The panel is the local alert receiver of the ICHU solution (report, Annex J, section J.3). It does not receive pushes: each poll is a `GET /api/v1/monitoring/alerts` to the Edge API, which holds the alerts raised from the collars. The panel keeps only the alerts that are still active, chooses one of four states and shows it.

Wokwi cannot simulate the I2C level converter and the 5 V supply of the prototype, so the diagram powers the LCD directly from the board (see [Hardware](#hardware)).

---

## Hardware

| Part | Interface | ESP32 pin | Notes |
| --- | --- | --- | --- |
| 16x2 LCD with I2C backpack | I2C (SDA, SCL) | GPIO21, GPIO22 | Address 0x27, powered from 5 V |
| Green LED | Digital output | GPIO25 | 220 Ω in series, state Normal |
| Yellow LED | Digital output | GPIO26 | 220 Ω in series, state Attention |
| Red LED | Digital output | GPIO27 | 220 Ω in series, state Urgent |
| Active buzzer KY-012 | Digital output | GPIO14 | Sounds while the pin is HIGH |

The prototype also needs an I2C level converter between the 3.3 V board and the 5 V LCD, and a regulated 5 V supply that shares ground with the board. Wokwi does not simulate either part, so the diagram connects the LCD to the board's 5 V pin.

---

## Project Structure

```text
ichu-alert-panel-embedded-app/
├── sketch.ino                 composition root: setup() and loop()
├── PanelConfig.h              identity, pins, intervals and the alert window
├── PanelSecrets.example.h     template for the git-ignored PanelSecrets.h
├── AlertStatus.h/.cpp         state rules: NoConnection, Normal, Attention, Urgent
├── EdgeAlertClient.h/.cpp     Wi-Fi and the GET of the alert listing
├── PanelClock.h/.cpp          NTP and the check that the clock is valid
├── StatusIndicator.h/.cpp     LEDs and buzzer, with the non-blocking alarm
├── StatusDisplay.h/.cpp       16x2 LCD, rewritten only when its text changes
├── diagram.json               Wokwi circuit
├── libraries.txt              Wokwi library list
├── wokwi.toml                 Wokwi for VS Code configuration
├── wokwi-project.txt          Wokwi project link (pending)
├── tests/alert_status_test.cpp host check of the state rules
├── CHANGELOG.md
├── CONTRIBUTING.md
├── LICENSE.md
└── README.md
```

The sources stay at the root so that the project also opens in the Wokwi web editor.

---

## Configuration

Copy `PanelSecrets.example.h` to `PanelSecrets.h` and set the values there. The copy is git-ignored, and a fresh clone builds with the example values, which target Wokwi.

- `kDeviceApiKey` must equal the `DEVELOPMENT_DEVICE_API_KEY` that the Edge API seeds for the display panel.
- `kWifiSsid` and `kWifiPassword` select the network. The ESP32 joins only 2.4 GHz networks.
- `kEdgeBaseUrl` points to the Edge API. The Wokwi value reaches the host through `host.wokwi.internal`. On hardware, set it to the PC LAN address, for example `http://192.168.1.20:5000`, and start the Edge API with `HOST=0.0.0.0` so that it listens on the LAN.

Edit `PanelConfig.h` for the rest:

- `kDeviceId` identifies the development panel, `bf6c7b5d-5c8a-4ba6-8fcc-6a7b8c9daeb5`.
- `kLcdAddress` is the I2C address of the LCD backpack.
- The LED and buzzer pins, the poll interval, the HTTP timeout, the stale limit, the 600 s alert window and the 30 s alarm are in the same file.

---

## Build

Every change must compile for `esp32:esp32:esp32` without warnings from the project files. arduino-cli expects the main file to match its folder name, so the sources are copied into a folder named `sketch` outside the repository:

```sh
arduino-cli core install esp32:esp32@3.3.4
arduino-cli lib install ArduinoJson "LiquidCrystal I2C"
mkdir -p ../build-src/sketch
cp *.ino *.h *.cpp ../build-src/sketch/
arduino-cli compile --fqbn esp32:esp32:esp32 --warnings all --output-dir build ../build-src/sketch
```

To flash a board, run `arduino-cli upload --port <port> --fqbn esp32:esp32:esp32 ../build-src/sketch`. The `build/` folder is ignored by Git.

The firmware uses 1081499 bytes of flash (82% of the default 1310720-byte application partition) and 50080 bytes of static RAM (15% of 327680 bytes) with the ESP32 core 3.3.4.

The LiquidCrystal I2C 1.1.2 library prints deprecation warnings for its `B00000001` binary-literal macros, and an `avr` architecture notice, when it is built for `esp32`. Those lines come from the library, not from the project files.

---

## Tests

`tests/alert_status_test.cpp` checks the state rules of `AlertStatus` on the host, without the Arduino core. From the repository root:

```sh
mkdir -p build
g++ -std=c++17 -Wall -Wextra -I. tests/alert_status_test.cpp AlertStatus.cpp -o build/alert_status_test
build/alert_status_test
```

The program prints `all alert status checks passed` and exits with 0 when every rule holds.

---

## Simulation in Wokwi

**Wokwi for VS Code.** `wokwi.toml` points to `build/sketch.ino.bin` and `build/sketch.ino.elf`, which the build above produces.

**Wokwi web editor.** The project link will be recorded in `wokwi-project.txt` once the team publishes it.

`diagram.json` wires the parts of the hardware table to the board. The LCD uses the I2C mode of the Wokwi part, whose default address is 0x27. The Wokwi buzzer sounds when its positive pin is driven HIGH.

**How to check the panel.** Start the Edge API locally with the development seed and run the collar simulator in fever mode from the `ichu-edge-service` folder:

```sh
python scripts/simulate_collar.py --mode fever
```

The Edge API raises a `Temperature/Critical` alert only when the collar has a synchronized assignment and rules (see the Edge API README). Within one poll the panel shows `URGENT` with `Temperature 9d4a` on the second row, the red LED lights and the buzzer sounds for 30 s.

---

## Edge Contract

The panel reads the alert listing of `ichu-edge-service`:

`GET {kEdgeBaseUrl}/api/v1/monitoring/alerts?device_id={kDeviceId}&limit=20` with the header `X-API-Key: {kDeviceApiKey}`.

| Response | Panel behavior |
| --- | --- |
| `200 OK` | Replaces the listing and restarts the stale timer. |
| `400` or `401` | Logged as a configuration error. The listing and the stale timer are not changed. |
| Other code | Logged with the code. The last listing is kept until the stale limit. |
| No Wi-Fi | No request is made. The last listing is kept until the stale limit. |

From each alert the panel reads `alert_id`, `device_id`, `type`, `severity` and `raised_at`. Only `Warning` and `Critical` alerts count: J.3 defines no state for `Informative`, so those are skipped. `raised_at` is read from its first 19 characters, so the fractions of a second are ignored.

---

## States

An alert is active when `now - raised_at` is at most 600 s. Until NTP sets the clock (before 2024-01-01 UTC), every alert the panel receives counts as active.

| State | Report name | Condition | LED | Buzzer | LCD row 1 | LCD row 2 |
| --- | --- | --- | --- | --- | --- | --- |
| `Normal` | Normal | Valid response without active alerts | Green | Off | `NORMAL` | `No active alerts` |
| `Attention` | Atención | The most severe active alert is a Warning | Yellow | Off | `ATTENTION` | `<Type> <tag>` |
| `Urgent` | Urgencia | At least one active Critical alert | Red | See below | `URGENT` | `<Type> <tag>` |
| `NoConnection` | Sin comunicación | No 200 response for more than 30 s, or none since boot | None | Off | `NO CONNECTION` | `Check edge/Wi-Fi` |

- The second row names the most severe active alert and, among equal severities, the newest one. It shows `<Type> <first four characters of device_id>`, for example `Temperature 9d4a`, cut to 16 characters.
- The buzzer pulses 1 s on and 1 s off for 30 s when the Urgent state shows an alert id that has not sounded yet. The same alert does not sound again, even while the state stays Urgent. Leaving Urgent turns the buzzer off at once.
- Only one LED is lit at a time. The LCD rewrites a row only when its text changes, padded with spaces to 16 characters.

---

## Serial Output

```text
[wifi] connecting to Wokwi-GUEST
[wifi] connected, IP <address>
[clock] NTP sync started
[status] URGENT
[alarm] buzzer started for <alert id>
[edge] alert listing failed (<code>), retrying
```

The `[status]` line appears only when the state changes. A failed poll prints its HTTP code, and a `400` or `401` answer prints a hint to check the device id and API key. The key and the password are never printed.

---

## Known Limitations

- The panel polls every 3 s instead of receiving pushes, so a new alert appears after up to one poll interval plus the request time. This does not meet the J.7 target of 1 s p95 from Edge to the panel.
- A poll blocks the loop for at most `kHttpTimeoutMs`, so the LED and buzzer pulses pause during that time.
- The panel does not remember the alert that sounded before a reboot, so a Critical alert that is still active sounds once more after a restart.
- Informative alerts do not change the state.
- The LCD has no text scrolling and there is no button to silence the buzzer.
- The buzzer trigger level and current draw of the KY-012 module are not verified; the wiring assumes HIGH sounds it.
- The level converter and the 5 V supply are not part of the simulation.
- The physical prototype has not been tested yet.

---

## License

See [LICENSE.md](LICENSE.md).
