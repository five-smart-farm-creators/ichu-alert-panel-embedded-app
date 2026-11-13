#include "StatusIndicator.h"

#include <Arduino.h>
#include <stdio.h>
#include <string.h>

#include "PanelConfig.h"

void StatusIndicator::begin() {
    pinMode(PanelConfig::kGreenLedPin, OUTPUT);
    pinMode(PanelConfig::kYellowLedPin, OUTPUT);
    pinMode(PanelConfig::kRedLedPin, OUTPUT);
    pinMode(PanelConfig::kBuzzerPin, OUTPUT);
}

void StatusIndicator::update(const StatusView& view, uint32_t nowMs) {
    // Only a Critical alert whose id differs from the last one that sounded starts a new alarm.
    if (view.state == PanelState::Urgent && strcmp(view.shownAlert->alertId, alarmedAlertId_) != 0) {
        snprintf(alarmedAlertId_, sizeof(alarmedAlertId_), "%s", view.shownAlert->alertId);
        alarmActive_ = true;
        alarmStartMs_ = nowMs;
        Serial.printf("[alarm] buzzer started for %s\n", alarmedAlertId_);
    }
    // Leaving Urgent silences the buzzer at once; an alarm itself lasts kBuzzerAlarmMs.
    if (view.state != PanelState::Urgent || nowMs - alarmStartMs_ >= PanelConfig::kBuzzerAlarmMs) {
        alarmActive_ = false;
    }
    const bool buzzerOn =
        alarmActive_ && ((nowMs - alarmStartMs_) / PanelConfig::kBuzzerPulseMs) % 2 == 0;

    digitalWrite(PanelConfig::kBuzzerPin, buzzerOn ? HIGH : LOW);
    digitalWrite(PanelConfig::kGreenLedPin, view.state == PanelState::Normal ? HIGH : LOW);
    digitalWrite(PanelConfig::kYellowLedPin, view.state == PanelState::Attention ? HIGH : LOW);
    digitalWrite(PanelConfig::kRedLedPin, view.state == PanelState::Urgent ? HIGH : LOW);
}
