#include "StatusDisplay.h"

#include <stdio.h>
#include <string.h>

namespace {

// Pads or cuts the text to 16 characters, so no part of an earlier text stays on the row.
void formatRow(const char* text, char (&row)[17]) {
    snprintf(row, sizeof(row), "%-16.16s", text);
}

// Second row of Attention and Urgent: "<Type> <device tag>", cut to fit the row.
void formatAlertRow(const AlertEntry& alert, char (&row)[17]) {
    char text[sizeof(alert.type) + sizeof(alert.deviceTag) + 1];
    snprintf(text, sizeof(text), "%s %s", alert.type, alert.deviceTag);
    formatRow(text, row);
}

}  // namespace

void StatusDisplay::begin() {
    lcd_.init();
    lcd_.backlight();
}

void StatusDisplay::show(const StatusView& view) {
    char top[17] = {};
    char bottom[17] = {};
    switch (view.state) {
        case PanelState::Normal:
            formatRow("NORMAL", top);
            formatRow("No active alerts", bottom);
            break;
        case PanelState::Attention:
            formatRow("ATTENTION", top);
            formatAlertRow(*view.shownAlert, bottom);
            break;
        case PanelState::Urgent:
            formatRow("URGENT", top);
            formatAlertRow(*view.shownAlert, bottom);
            break;
        case PanelState::NoConnection:
            formatRow("NO CONNECTION", top);
            formatRow("Check edge/Wi-Fi", bottom);
            break;
    }
    writeRow(0, top);
    writeRow(1, bottom);
}

// Writes only when the text differs from what the LCD already shows, which avoids flicker.
void StatusDisplay::writeRow(uint8_t row, const char* text) {
    if (strcmp(text, rows_[row]) == 0) {
        return;
    }
    lcd_.setCursor(0, row);
    lcd_.print(text);
    snprintf(rows_[row], sizeof(rows_[row]), "%s", text);
}
