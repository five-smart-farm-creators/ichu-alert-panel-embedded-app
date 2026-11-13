#include "PanelClock.h"

#include <Arduino.h>
#include <time.h>

#include "PanelConfig.h"

namespace {

// 2024-01-01T00:00:00Z: an earlier clock has never been set by NTP.
constexpr int64_t kEarliestValidEpoch = 1704067200;

}  // namespace

void PanelClock::update(bool wifiConnected) {
    if (wifiConnected && !ntpRequested_) {
        configTime(0, 0, PanelConfig::kNtpServer);
        ntpRequested_ = true;
        Serial.println("[clock] NTP sync started");
    }
}

bool PanelClock::isValid() const {
    return epochNow() >= kEarliestValidEpoch;
}

int64_t PanelClock::epochNow() const {
    return static_cast<int64_t>(time(nullptr));
}
