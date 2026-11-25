#ifndef BUZZER_ALARM_H
#define BUZZER_ALARM_H

#include <Arduino.h>
#include <stdint.h>

#include <ModestIoT.h>

#include "AlertStatus.h"
#include "PanelConfig.h"

// Sounds the buzzer for kBuzzerAlarmMs when a Critical alert first becomes the shown alert.
class BuzzerAlarm {
public:
    void update(Buzzer& buzzer, const StatusView& view, uint32_t nowMs);

private:
    bool active_ = false;
    uint32_t startMs_ = 0;
    // The last alert that sounded; its id does not start a second alarm while the state stays Urgent.
    char alarmedAlertId_[PanelConfig::kAlertIdCapacity] = "";
};

#endif  // BUZZER_ALARM_H
