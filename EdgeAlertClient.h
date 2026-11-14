#ifndef EDGE_ALERT_CLIENT_H
#define EDGE_ALERT_CLIENT_H

#include <stddef.h>

#include "AlertStatus.h"

// Wi-Fi station and the GET of the alert listing; fetch() blocks for at most kHttpTimeoutMs.
class EdgeAlertClient {
public:
    void begin();
    void poll();
    bool isAvailable() const;
    // Writes the listing only for a readable 200 response, so a failed poll keeps the last one.
    bool fetch(AlertEntry* alerts, size_t capacity, size_t& count);

private:
    bool wasConnected_ = false;
};

#endif  // EDGE_ALERT_CLIENT_H
