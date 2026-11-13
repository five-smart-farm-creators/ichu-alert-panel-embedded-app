#ifndef STATUS_DISPLAY_H
#define STATUS_DISPLAY_H

#include <LiquidCrystal_I2C.h>
#include <stdint.h>

#include "AlertStatus.h"
#include "PanelConfig.h"

// Shows the state on the 16x2 LCD; a row is rewritten only when its text changes.
class StatusDisplay {
public:
    void begin();
    void show(const StatusView& view);

private:
    void writeRow(uint8_t row, const char* text);

    LiquidCrystal_I2C lcd_{PanelConfig::kLcdAddress, 16, 2};
    char rows_[2][17] = {};
};

#endif  // STATUS_DISPLAY_H
