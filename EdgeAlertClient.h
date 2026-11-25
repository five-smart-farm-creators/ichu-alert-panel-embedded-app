#ifndef EDGE_ALERT_CLIENT_H
#define EDGE_ALERT_CLIENT_H

#include <stddef.h>

#include "AlertStatus.h"

// Reads the alert listing with a plain HTTP GET: HttpGatewayClient keeps only 511 response bytes,
// and two alerts of the listing already need about 830.
class EdgeAlertClient {
public:
    // Writes the alerts of a readable 200 response and leaves them untouched otherwise.
    bool fetch(AlertEntry* alerts, size_t capacity, size_t& count) const;
};

#endif  // EDGE_ALERT_CLIENT_H
