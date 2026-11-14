#ifndef PANEL_SECRETS_EXAMPLE_H
#define PANEL_SECRETS_EXAMPLE_H

// Copy to PanelSecrets.h (git-ignored) and set the real values.
namespace PanelSecrets {

// Must equal the DEVELOPMENT_DEVICE_API_KEY that the Edge API seeds for the display panel.
constexpr char kDeviceApiKey[] = "development-device-key";
// Wokwi reaches the host through this gateway name; hardware needs the PC LAN address.
constexpr char kWifiSsid[] = "Wokwi-GUEST";
constexpr char kWifiPassword[] = "";
constexpr char kEdgeBaseUrl[] = "http://host.wokwi.internal:5000";

}  // namespace PanelSecrets

#endif  // PANEL_SECRETS_EXAMPLE_H
