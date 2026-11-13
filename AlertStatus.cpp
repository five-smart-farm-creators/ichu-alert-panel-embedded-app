#include "AlertStatus.h"

namespace {

// Alerts stay active for kAlertWindowSeconds; before the clock is set, every received alert counts.
bool isActive(const AlertEntry& alert, const StatusInput& input) {
    return !input.clockValid || input.now - alert.raisedAt <= PanelConfig::kAlertWindowSeconds;
}

// Higher severity wins; between equal severities the newer alert is the one to name.
bool outranks(const AlertEntry& candidate, const AlertEntry& current) {
    if (candidate.severity != current.severity) {
        return candidate.severity > current.severity;
    }
    return candidate.raisedAt > current.raisedAt;
}

}  // namespace

StatusView evaluateStatus(const StatusInput& input) {
    if (!input.hasResponse || input.responseAgeMs > PanelConfig::kStaleAfterMs) {
        return {PanelState::NoConnection, nullptr};
    }
    const AlertEntry* shown = nullptr;
    for (size_t i = 0; i < input.alertCount; ++i) {
        const AlertEntry& alert = input.alerts[i];
        if (isActive(alert, input) && (shown == nullptr || outranks(alert, *shown))) {
            shown = &alert;
        }
    }
    if (shown == nullptr) {
        return {PanelState::Normal, nullptr};
    }
    const PanelState state =
        shown->severity == AlertSeverity::Critical ? PanelState::Urgent : PanelState::Attention;
    return {state, shown};
}
