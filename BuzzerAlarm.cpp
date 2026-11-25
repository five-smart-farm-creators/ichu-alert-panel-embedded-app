#include "BuzzerAlarm.h"

#include <Arduino.h>
#include <stdio.h>
#include <string.h>

#include "PanelConfig.h"

void BuzzerAlarm::update(Buzzer& buzzer, const StatusView& view, const uint32_t nowMs) {
    // Only a Critical alert whose id differs from the last one that sounded starts a new alarm.
    if (view.state == PanelState::Urgent && strcmp(view.shownAlert->alertId, alarmedAlertId_) != 0) {
        snprintf(alarmedAlertId_, sizeof(alarmedAlertId_), "%s", view.shownAlert->alertId);
        active_ = true;
        startMs_ = nowMs;
        Serial.printf("[alarm] buzzer started for %s\n", alarmedAlertId_);
    }
    // Leaving Urgent silences the buzzer at once; an alarm itself lasts kBuzzerAlarmMs.
    if (view.state != PanelState::Urgent || nowMs - startMs_ >= PanelConfig::kBuzzerAlarmMs) {
        active_ = false;
    }
    // A repeated TURN_ON restarts the tone, so a command is sent only when the output changes.
    const bool sounding = active_ && ((nowMs - startMs_) / PanelConfig::kBuzzerPulseMs) % 2 == 0;
    if (sounding != buzzer.isActive()) {
        buzzer.handle(sounding ? Buzzer::TURN_ON_COMMAND : Buzzer::TURN_OFF_COMMAND);
    }
}
