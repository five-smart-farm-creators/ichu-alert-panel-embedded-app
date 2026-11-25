#ifndef PANEL_CONFIG_H
#define PANEL_CONFIG_H

#include <stddef.h>
#include <stdint.h>

// A fresh clone builds with the development values until PanelSecrets.h exists.
#if __has_include("PanelSecrets.h")
#include "PanelSecrets.h"
#else
#include "PanelSecrets.example.h"
#endif

// Values that differ between the simulation and the hardware prototype live here.
namespace PanelConfig {

// Development panel that the Edge API seeds with DEVELOPMENT_DEVICE_API_KEY.
constexpr char kDeviceId[] = "bf6c7b5d-5c8a-4ba6-8fcc-6a7b8c9daeb5";
constexpr char kAlertsPath[] = "/api/v1/monitoring/alerts";
constexpr char kNtpServer[] = "pool.ntp.org";
// The framework embeds this version in telemetry envelopes, which the panel never sends.
constexpr char kFirmwareVersion[] = "0.2.0";

// The framework LCD and Wire use the ESP32 default I2C bus (SDA GPIO21, SCL GPIO22).
// Many backpacks ship with 0x3F; change the address when the LCD does not answer.
constexpr uint8_t kLcdAddress = 0x27;
constexpr uint8_t kGreenLedPin = 25;
constexpr uint8_t kYellowLedPin = 26;
constexpr uint8_t kRedLedPin = 27;
constexpr uint8_t kBuzzerPin = 14;

constexpr uint32_t kPollIntervalMs = 3000;
constexpr uint32_t kHttpTimeoutMs = 5000;
// The sampler task refreshes the outputs at this period, so an LED or buzzer change waits at most this long.
constexpr uint32_t kRefreshIntervalMs = 250;
// Past this age the last listing is no longer trusted, so the panel shows NO CONNECTION.
constexpr uint32_t kStaleAfterMs = 30000;
// NTP is requested again at this period while the framework reports no synchronized clock.
constexpr uint32_t kClockRetryMs = 60000;
// J.3: an alert is active for this long after its raised_at.
constexpr int64_t kAlertWindowSeconds = 600;

constexpr uint32_t kBuzzerPulseMs = 1000;
constexpr uint32_t kBuzzerAlarmMs = 30000;

// The query asks for this many alerts, so the listing never holds more.
constexpr size_t kAlertLimit = 20;
// A UUID has 36 characters; the extra byte holds the terminator.
constexpr size_t kAlertIdCapacity = 37;
// The LCD shows 16 characters per row, so a longer type name cannot be displayed anyway.
constexpr size_t kAlertTypeCapacity = 16;

}  // namespace PanelConfig

#endif  // PANEL_CONFIG_H
