#include "EdgeAlertClient.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <stdio.h>
#include <string.h>

#include "PanelConfig.h"

namespace {

// Days since 1970-01-01 for a proleptic Gregorian date; mktime would depend on the local TZ.
int64_t daysFromCivil(int64_t year, int64_t month, int64_t day) {
    year -= month <= 2 ? 1 : 0;
    const int64_t era = (year >= 0 ? year : year - 399) / 400;
    const int64_t yearOfEra = year - era * 400;
    const int64_t dayOfYear = (153 * (month > 2 ? month - 3 : month + 9) + 2) / 5 + day - 1;
    const int64_t dayOfEra = yearOfEra * 365 + yearOfEra / 4 - yearOfEra / 100 + dayOfYear;
    return era * 146097 + dayOfEra - 719468;
}

// Only the first 19 characters matter: fractions of a second and the Z suffix are dropped.
bool parseRaisedAt(const char* text, int64_t& epochSeconds) {
    int year = 0;
    int month = 0;
    int day = 0;
    int hour = 0;
    int minute = 0;
    int second = 0;
    if (sscanf(text, "%4d-%2d-%2dT%2d:%2d:%2d", &year, &month, &day, &hour, &minute, &second) != 6) {
        return false;
    }
    epochSeconds = daysFromCivil(year, month, day) * 86400 + hour * 3600 + minute * 60 + second;
    return true;
}

// The Edge API also sends Informative alerts, which J.3 has no panel state for, so they are skipped.
bool readAlert(JsonObject item, AlertEntry& entry) {
    const char* severity = item["severity"].as<const char*>();
    const char* alertId = item["alert_id"].as<const char*>();
    const char* type = item["type"].as<const char*>();
    const char* deviceId = item["device_id"].as<const char*>();
    const char* raisedText = item["raised_at"].as<const char*>();
    if (severity == nullptr || alertId == nullptr || type == nullptr || deviceId == nullptr ||
        raisedText == nullptr) {
        return false;
    }
    if (strcmp(severity, "Critical") == 0) {
        entry.severity = AlertSeverity::Critical;
    } else if (strcmp(severity, "Warning") == 0) {
        entry.severity = AlertSeverity::Warning;
    } else {
        return false;
    }
    if (!parseRaisedAt(raisedText, entry.raisedAt)) {
        return false;
    }
    snprintf(entry.alertId, sizeof(entry.alertId), "%s", alertId);
    snprintf(entry.type, sizeof(entry.type), "%s", type);
    snprintf(entry.deviceTag, sizeof(entry.deviceTag), "%.4s", deviceId);
    return true;
}

}  // namespace

void EdgeAlertClient::begin() {
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    WiFi.begin(PanelSecrets::kWifiSsid, PanelSecrets::kWifiPassword);
    Serial.printf("[wifi] connecting to %s\n", PanelSecrets::kWifiSsid);
}

void EdgeAlertClient::poll() {
    const bool connected = isAvailable();
    if (connected == wasConnected_) {
        return;
    }
    wasConnected_ = connected;
    if (connected) {
        Serial.printf("[wifi] connected, IP %s\n", WiFi.localIP().toString().c_str());
    } else {
        Serial.println("[wifi] disconnected");
    }
}

bool EdgeAlertClient::isAvailable() const {
    return WiFi.status() == WL_CONNECTED;
}

bool EdgeAlertClient::fetch(AlertEntry* alerts, size_t capacity, size_t& count) {
    if (!isAvailable()) {
        return false;
    }
    const String url = String(PanelSecrets::kEdgeBaseUrl) + PanelConfig::kAlertsPath +
                       "?device_id=" + PanelConfig::kDeviceId +
                       "&limit=" + String(PanelConfig::kAlertLimit);
    HTTPClient http;
    http.begin(url);
    http.setConnectTimeout(static_cast<int32_t>(PanelConfig::kHttpTimeoutMs));
    http.setTimeout(static_cast<uint16_t>(PanelConfig::kHttpTimeoutMs));
    http.addHeader("X-API-Key", PanelSecrets::kDeviceApiKey);
    const int status = http.GET();
    String body;
    if (status == 200) {
        body = http.getString();
    }
    http.end();

    if (status != 200) {
        // 400 and 401 are configuration errors, so the message names the settings and never the key.
        if (status == 400 || status == 401) {
            Serial.printf("[edge] alert listing rejected (%d), check the device id and API key\n", status);
        } else {
            Serial.printf("[edge] alert listing failed (%d), retrying\n", status);
        }
        return false;
    }

    JsonDocument filter;
    filter["alerts"][0]["alert_id"] = true;
    filter["alerts"][0]["device_id"] = true;
    filter["alerts"][0]["type"] = true;
    filter["alerts"][0]["severity"] = true;
    filter["alerts"][0]["raised_at"] = true;

    JsonDocument document;
    const DeserializationError error =
        deserializeJson(document, body, DeserializationOption::Filter(filter));
    if (error) {
        Serial.printf("[edge] alert listing unreadable: %s\n", error.c_str());
        return false;
    }

    size_t stored = 0;
    for (JsonObject item : document["alerts"].as<JsonArray>()) {
        if (stored == capacity) {
            break;
        }
        AlertEntry entry{};
        if (readAlert(item, entry)) {
            alerts[stored++] = entry;
        }
    }
    count = stored;
    return true;
}
