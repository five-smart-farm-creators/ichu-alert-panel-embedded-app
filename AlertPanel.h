#ifndef ALERT_PANEL_H
#define ALERT_PANEL_H

#include <Arduino.h>
#include <freertos/semphr.h>
#include <stddef.h>
#include <stdint.h>

#include <ModestIoT.h>

#include "AlertStatus.h"
#include "BuzzerAlarm.h"
#include "EdgeAlertClient.h"
#include "PanelConfig.h"

// Mediator of the panel: the sampler task drives the outputs from the latest listing, and the
// network task polls the Edge API and publishes the listing under a mutex.
class AlertPanel : public Device {
public:
    explicit AlertPanel(WiFiConnectivityDriver& wifiDriver);
    ~AlertPanel() override;

    void on(Event event) override;

protected:
    void maintainNetworkServices() const override;

private:
    // An alert listing and the time the network task received it.
    struct Listing {
        AlertEntry alerts[PanelConfig::kAlertLimit];
        size_t count;
        bool received;
        uint32_t receivedAtMs;
    };

    static constexpr int REFRESH_EVENT_IDENTIFIER = 1000;  // application range of the framework

    void refreshOutputs(uint32_t nowMs);
    void setStateLeds(PanelState state);
    void showStatusText(const StatusView& view);
    void keepClockSynchronized(uint32_t nowMs) const;
    void pollAlertListing(uint32_t nowMs) const;

    Led greenLed_;
    Led yellowLed_;
    Led redLed_;
    Buzzer buzzer_;
    CharacterLcdDisplay lcd_;
    WiFiConnectivityDriver& wifi_;
    EdgeAlertClient edgeClient_;
    BuzzerAlarm alarm_;
    SemaphoreHandle_t listingMutex_;
    PanelState reportedState_;
    char shownRows_[2][17] = {};

    // Written by the network task and copied by the sampler task, both under listingMutex_.
    mutable Listing shared_{};

    // Network task only.
    mutable Listing fetched_{};
    mutable uint32_t lastPollMs_ = 0;
    mutable bool wifiConnected_ = false;
    mutable bool clockAttempted_ = false;
    mutable uint32_t lastClockAttemptMs_ = 0;

    // Sampler task only.
    Listing sampled_{};
};

#endif  // ALERT_PANEL_H
