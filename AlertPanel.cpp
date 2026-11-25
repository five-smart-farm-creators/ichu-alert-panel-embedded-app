#include "AlertPanel.h"

#include <Arduino.h>
#include <stdio.h>
#include <string.h>

#include "PanelConfig.h"

namespace {

const char* stateName(const PanelState state) {
    switch (state) {
        case PanelState::Normal:
            return "NORMAL";
        case PanelState::Attention:
            return "ATTENTION";
        case PanelState::Urgent:
            return "URGENT";
        case PanelState::NoConnection:
            return "NO CONNECTION";
    }
    return "";
}

// The LCD shows 16 characters per row; the framework pads a shorter row with spaces.
void formatRow(const char* text, char (&row)[17]) {
    snprintf(row, sizeof(row), "%.16s", text);
}

// Second row of Attention and Urgent: "<Type> <device tag>".
void formatAlertRow(const AlertEntry& alert, char (&row)[17]) {
    char text[sizeof(alert.type) + sizeof(alert.deviceTag) + 1];
    snprintf(text, sizeof(text), "%s %s", alert.type, alert.deviceTag);
    formatRow(text, row);
}

}  // namespace

AlertPanel::AlertPanel(WiFiConnectivityDriver& wifiDriver)
    : Device(PanelConfig::kRefreshIntervalMs),
      greenLed_(PanelConfig::kGreenLedPin, false),
      yellowLed_(PanelConfig::kYellowLedPin, false),
      redLed_(PanelConfig::kRedLedPin, false),
      buzzer_(PanelConfig::kBuzzerPin),
      lcd_(PanelConfig::kLcdAddress, 16, 2),
      wifi_(wifiDriver),
      listingMutex_(xSemaphoreCreateMutex()),
      reportedState_(PanelState::NoConnection) {
    Device::setFirmwareVersion(PanelConfig::kFirmwareVersion);
    // Slots must be registered before the engine starts.
    appendSensorToScheduler(this, REFRESH_EVENT_IDENTIFIER, PanelConfig::kRefreshIntervalMs, 0);
    initializeAsynchronousEngine();
}

AlertPanel::~AlertPanel() {
    // The engine must stop before the mutex that its network task uses is deleted.
    stopAsynchronousEngine();
    vSemaphoreDelete(listingMutex_);
}

void AlertPanel::on(Event event) {
    if (event.identifier == REFRESH_EVENT_IDENTIFIER) {
        refreshOutputs(millis());
    }
}

void AlertPanel::maintainNetworkServices() const {
    const uint32_t nowMs = millis();
    // connect() starts an attempt only after the driver's reconnect interval, so calling it on every pass is cheap.
    wifi_.connect();
    const bool connected = wifi_.isConnected();
    if (connected != wifiConnected_) {
        wifiConnected_ = connected;
        if (connected) {
            Serial.printf("[wifi] connected, IP %s\n", wifi_.getLocalIpAddress().toString().c_str());
        } else {
            Serial.println("[wifi] disconnected");
        }
    }
    if (!connected) {
        return;
    }
    keepClockSynchronized(nowMs);
    // Subtraction keeps the interval correct when millis() wraps around.
    if (nowMs - lastPollMs_ >= PanelConfig::kPollIntervalMs) {
        lastPollMs_ = nowMs;
        pollAlertListing(nowMs);
    }
}

void AlertPanel::keepClockSynchronized(const uint32_t nowMs) const {
    if (TimeSynchronizationDriver::isSynchronized()) {
        return;
    }
    if (clockAttempted_ && nowMs - lastClockAttemptMs_ < PanelConfig::kClockRetryMs) {
        return;
    }
    clockAttempted_ = true;
    lastClockAttemptMs_ = nowMs;
    Serial.println("[clock] NTP sync started");
    // initialize() waits up to 5 s for NTP and never retries by itself, so it is called again here.
    TimeSynchronizationDriver::initialize("UTC0", PanelConfig::kNtpServer);
}

void AlertPanel::pollAlertListing(const uint32_t nowMs) const {
    size_t count = 0;
    if (!edgeClient_.fetch(fetched_.alerts, PanelConfig::kAlertLimit, count)) {
        return;
    }
    fetched_.count = count;
    fetched_.received = true;
    fetched_.receivedAtMs = nowMs;
    xSemaphoreTake(listingMutex_, portMAX_DELAY);
    shared_ = fetched_;
    xSemaphoreGive(listingMutex_);
}

void AlertPanel::refreshOutputs(const uint32_t nowMs) {
    xSemaphoreTake(listingMutex_, portMAX_DELAY);
    sampled_ = shared_;
    xSemaphoreGive(listingMutex_);

    StatusInput input{};
    input.alerts = sampled_.alerts;
    input.alertCount = sampled_.count;
    input.hasResponse = sampled_.received;
    input.responseAgeMs = nowMs - sampled_.receivedAtMs;
    input.clockValid = TimeSynchronizationDriver::isSynchronized();
    input.now = static_cast<int64_t>(TimeSynchronizationDriver::getCurrentTimestamp());
    const StatusView view = evaluateStatus(input);

    // The LEDs switch before the status line is printed, so a test that reads the line sees the lit LED.
    setStateLeds(view.state);
    if (view.state != reportedState_) {
        reportedState_ = view.state;
        Serial.printf("[status] %s\n", stateName(view.state));
    }
    showStatusText(view);
    alarm_.update(buzzer_, view, nowMs);
}

void AlertPanel::setStateLeds(const PanelState state) {
    greenLed_.handle(state == PanelState::Normal ? Led::TURN_ON_COMMAND : Led::TURN_OFF_COMMAND);
    yellowLed_.handle(state == PanelState::Attention ? Led::TURN_ON_COMMAND : Led::TURN_OFF_COMMAND);
    redLed_.handle(state == PanelState::Urgent ? Led::TURN_ON_COMMAND : Led::TURN_OFF_COMMAND);
}

void AlertPanel::showStatusText(const StatusView& view) {
    char top[17] = {};
    char bottom[17] = {};
    switch (view.state) {
        case PanelState::Normal:
            formatRow("NORMAL", top);
            formatRow("No active alerts", bottom);
            break;
        case PanelState::Attention:
            formatRow("ATTENTION", top);
            formatAlertRow(*view.shownAlert, bottom);
            break;
        case PanelState::Urgent:
            formatRow("URGENT", top);
            formatAlertRow(*view.shownAlert, bottom);
            break;
        case PanelState::NoConnection:
            formatRow("NO CONNECTION", top);
            formatRow("Check edge/Wi-Fi", bottom);
            break;
    }
    // The framework rewrites both rows together, so the LCD is updated only when a row changes.
    if (strcmp(top, shownRows_[0]) == 0 && strcmp(bottom, shownRows_[1]) == 0) {
        return;
    }
    snprintf(shownRows_[0], sizeof(shownRows_[0]), "%s", top);
    snprintf(shownRows_[1], sizeof(shownRows_[1]), "%s", bottom);
    lcd_.setLineBuffer(0, top);
    lcd_.setLineBuffer(1, bottom);
    lcd_.handle(CharacterLcdDisplay::UPDATE_TEXT_COMMAND);
}
