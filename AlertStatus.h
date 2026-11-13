#ifndef ALERT_STATUS_H
#define ALERT_STATUS_H

#include <stddef.h>
#include <stdint.h>

#include "PanelConfig.h"

enum class AlertSeverity : uint8_t { Warning, Critical };

// One alert of the listing, reduced to the fields the panel shows.
struct AlertEntry {
    char alertId[PanelConfig::kAlertIdCapacity];
    char type[PanelConfig::kAlertTypeCapacity];
    char deviceTag[5];  // first four characters of device_id plus the terminator
    AlertSeverity severity;
    int64_t raisedAt;  // Unix seconds, UTC
};

enum class PanelState : uint8_t { Normal, Attention, Urgent, NoConnection };

struct StatusInput {
    const AlertEntry* alerts;
    size_t alertCount;
    bool hasResponse;  // false until the first 200 response after boot
    // Unsigned subtraction in the caller keeps this correct when millis() wraps around.
    uint32_t responseAgeMs;
    // Before NTP sets the clock, every received alert counts as active.
    bool clockValid;
    int64_t now;  // Unix seconds, UTC
};

struct StatusView {
    PanelState state;
    // Named on the second LCD row; nullptr for Normal and NoConnection.
    const AlertEntry* shownAlert;
};

// Applies J.3 to one listing: NoConnection first, then the most severe and newest active alert.
StatusView evaluateStatus(const StatusInput& input);

#endif  // ALERT_STATUS_H
