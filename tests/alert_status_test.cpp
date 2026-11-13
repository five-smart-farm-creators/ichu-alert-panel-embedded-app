// Host check of the J.3 state rules, without the Arduino core.
// Build from the repository root: g++ -std=c++17 -Wall -Wextra -I. tests/alert_status_test.cpp AlertStatus.cpp
#include <stdio.h>
#include <string.h>

#include "AlertStatus.h"

namespace {

int failures = 0;

void check(bool passed, const char* name) {
    if (!passed) {
        ++failures;
        printf("FAIL: %s\n", name);
    }
}

// A clock value after 2024-01-01, so alerts are judged against their raised_at.
constexpr int64_t kNow = 1800000000;

AlertEntry makeAlert(const char* id, AlertSeverity severity, int64_t raisedAt) {
    AlertEntry alert{};
    snprintf(alert.alertId, sizeof(alert.alertId), "%s", id);
    snprintf(alert.type, sizeof(alert.type), "%s", "Temperature");
    snprintf(alert.deviceTag, sizeof(alert.deviceTag), "%s", "9d4a");
    alert.severity = severity;
    alert.raisedAt = raisedAt;
    return alert;
}

StatusInput makeInput(const AlertEntry* alerts, size_t count) {
    StatusInput input{};
    input.alerts = alerts;
    input.alertCount = count;
    input.hasResponse = true;
    input.responseAgeMs = 0;
    input.clockValid = true;
    input.now = kNow;
    return input;
}

}  // namespace

int main() {
    StatusInput neverResponded = makeInput(nullptr, 0);
    neverResponded.hasResponse = false;
    check(evaluateStatus(neverResponded).state == PanelState::NoConnection, "no response since boot");

    StatusInput stale = makeInput(nullptr, 0);
    stale.responseAgeMs = PanelConfig::kStaleAfterMs + 1;
    check(evaluateStatus(stale).state == PanelState::NoConnection, "response older than the stale limit");

    StatusInput atLimit = makeInput(nullptr, 0);
    atLimit.responseAgeMs = PanelConfig::kStaleAfterMs;
    check(evaluateStatus(atLimit).state == PanelState::Normal, "response at the stale limit is still trusted");

    check(evaluateStatus(makeInput(nullptr, 0)).state == PanelState::Normal, "no alerts is Normal");

    const AlertEntry warning = makeAlert("warning", AlertSeverity::Warning, kNow - 60);
    const AlertEntry critical = makeAlert("critical", AlertSeverity::Critical, kNow - 120);
    const AlertEntry olderCritical = makeAlert("older", AlertSeverity::Critical, kNow - 300);

    const AlertEntry warningOnly[] = {warning};
    StatusView view = evaluateStatus(makeInput(warningOnly, 1));
    check(view.state == PanelState::Attention && view.shownAlert == &warningOnly[0], "warning is Attention");

    const AlertEntry mixed[] = {warning, critical};
    view = evaluateStatus(makeInput(mixed, 2));
    check(view.state == PanelState::Urgent && view.shownAlert == &mixed[1], "critical outranks warning");

    const AlertEntry twoCritical[] = {olderCritical, critical};
    view = evaluateStatus(makeInput(twoCritical, 2));
    check(view.shownAlert == &twoCritical[1], "the newer of two critical alerts is named");

    const AlertEntry onTheWindowEdge[] = {makeAlert("edge", AlertSeverity::Critical, kNow - 600)};
    check(evaluateStatus(makeInput(onTheWindowEdge, 1)).state == PanelState::Urgent,
          "alert exactly 600 s old is active");

    const AlertEntry expired[] = {makeAlert("expired", AlertSeverity::Critical, kNow - 601)};
    check(evaluateStatus(makeInput(expired, 1)).state == PanelState::Normal,
          "alert older than 600 s is ignored");

    StatusInput unsynced = makeInput(expired, 1);
    unsynced.clockValid = false;
    unsynced.now = 0;
    check(evaluateStatus(unsynced).state == PanelState::Urgent, "every alert is active before NTP");

    if (failures == 0) {
        printf("all alert status checks passed\n");
    }
    return failures == 0 ? 0 : 1;
}
