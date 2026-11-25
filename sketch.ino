// Composition root: creates the framework Wi-Fi driver and the AlertPanel mediator that owns the outputs.
#include <Arduino.h>
#include <ModestIoT.h>
#include <Wire.h>

#include "AlertPanel.h"
#include "PanelConfig.h"

void setup() {
    Serial.begin(115200);
    // The framework LCD writes through Wire and does not start the bus itself.
    Wire.begin();
    Serial.printf("[wifi] connecting to %s\n", PanelSecrets::kWifiSsid);
    // Function-local statics live for the whole run, because the tasks they start never stop.
    static WiFiConnectivityDriver wifiDriver(PanelSecrets::kWifiSsid, PanelSecrets::kWifiPassword);
    static AlertPanel alertPanel(wifiDriver);
}

void loop() {
    // The sampler and network tasks do the work in the background; loop() only yields.
    vTaskDelay(pdMS_TO_TICKS(60000));
}
