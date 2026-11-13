#ifndef PANEL_CLOCK_H
#define PANEL_CLOCK_H

#include <stdint.h>

// UTC time from NTP; the clock counts as valid once NTP has set a date after 2024-01-01.
class PanelClock {
public:
    void update(bool wifiConnected);
    bool isValid() const;
    int64_t epochNow() const;

private:
    bool ntpRequested_ = false;
};

#endif  // PANEL_CLOCK_H
