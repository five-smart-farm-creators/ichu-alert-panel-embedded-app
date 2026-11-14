// Composition root: polls the alert listing on a fixed interval and updates the outputs each loop.
#include <Arduino.h>

#include "AlertStatus.h"
#include "EdgeAlertClient.h"
#include "PanelClock.h"
#include "PanelConfig.h"
#include "StatusDisplay.h"
#include "StatusIndicator.h"

static EdgeAlertClient edgeClient;
static PanelClock panelClock;
static StatusDisplay statusDisplay;
static StatusIndicator statusIndicator;
static AlertEntry alerts[PanelConfig::kAlertLimit];
static size_t alertCount = 0;
static bool hasResponse = false;
static uint32_t lastResponseMs = 0;
static uint32_t lastPollMs = 0;
static PanelState reportedState = PanelState::NoConnection;

static const char* stateName(PanelState state) {
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

void setup() {
    Serial.begin(115200);
    statusIndicator.begin();
    statusDisplay.begin();
    edgeClient.begin();
}

void loop() {
    const uint32_t now = millis();
    edgeClient.poll();
    panelClock.update(edgeClient.isAvailable());

    // Subtraction keeps the interval correct when millis() wraps around.
    if (now - lastPollMs >= PanelConfig::kPollIntervalMs) {
        lastPollMs = now;
        size_t count = 0;
        if (edgeClient.fetch(alerts, PanelConfig::kAlertLimit, count)) {
            alertCount = count;
            hasResponse = true;
            lastResponseMs = now;
        }
    }

    StatusInput input{};
    input.alerts = alerts;
    input.alertCount = alertCount;
    input.hasResponse = hasResponse;
    input.responseAgeMs = now - lastResponseMs;
    input.clockValid = panelClock.isValid();
    input.now = panelClock.epochNow();
    const StatusView view = evaluateStatus(input);

    // Logs transitions only, so the Serial monitor shows each change once.
    if (view.state != reportedState) {
        reportedState = view.state;
        Serial.printf("[status] %s\n", stateName(view.state));
    }
    statusDisplay.show(view);
    statusIndicator.update(view, now);
}
