#ifndef STATUS_INDICATOR_H
#define STATUS_INDICATOR_H

#include <stdint.h>

#include "AlertStatus.h"

// Lights one LED per state and runs the buzzer alarm without blocking loop().
class StatusIndicator {
public:
    void begin();
    void update(const StatusView& view, uint32_t nowMs);

private:
    bool alarmActive_ = false;
    uint32_t alarmStartMs_ = 0;
    // The last Critical alert that sounded; repeating its id does not start a new alarm.
    char alarmedAlertId_[PanelConfig::kAlertIdCapacity] = "";
};

#endif  // STATUS_INDICATOR_H
