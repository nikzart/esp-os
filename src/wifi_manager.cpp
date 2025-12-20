#include "wifi_manager.h"
#include <HTTPClient.h>
#include "config.h"

Preferences WiFiManager::prefs;
WiFiNetwork WiFiManager::scanResults[MAX_SCAN_RESULTS];
int WiFiManager::scanCount = 0;
char WiFiManager::savedSSIDs[MAX_SAVED_NETWORKS][33] = {0};
char WiFiManager::savedPasswords[MAX_SAVED_NETWORKS][65] = {0};
int WiFiManager::savedCount = 0;
char WiFiManager::currentSSID[33] = {0};

void WiFiManager::init() {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    loadSavedNetworks();
}

void WiFiManager::loadSavedNetworks() {
    prefs.begin(NVS_NAMESPACE, true);  // Read-only

    savedCount = prefs.getInt("wifi_count", 0);
    if (savedCount > MAX_SAVED_NETWORKS) savedCount = MAX_SAVED_NETWORKS;

    for (int i = 0; i < savedCount; i++) {
        char keySSID[16], keyPass[16];
        snprintf(keySSID, sizeof(keySSID), "wifi_ssid_%d", i);
        snprintf(keyPass, sizeof(keyPass), "wifi_pass_%d", i);

        prefs.getString(keySSID, savedSSIDs[i], sizeof(savedSSIDs[i]));
        prefs.getString(keyPass, savedPasswords[i], sizeof(savedPasswords[i]));
    }

    prefs.end();
}

void WiFiManager::saveSavedNetworks() {
    prefs.begin(NVS_NAMESPACE, false);  // Read-write

    prefs.putInt("wifi_count", savedCount);

    for (int i = 0; i < savedCount; i++) {
        char keySSID[16], keyPass[16];
        snprintf(keySSID, sizeof(keySSID), "wifi_ssid_%d", i);
        snprintf(keyPass, sizeof(keyPass), "wifi_pass_%d", i);

        prefs.putString(keySSID, savedSSIDs[i]);
        prefs.putString(keyPass, savedPasswords[i]);
    }

    prefs.end();
}

int WiFiManager::scan() {
    scanCount = WiFi.scanNetworks();
    if (scanCount > MAX_SCAN_RESULTS) scanCount = MAX_SCAN_RESULTS;

    for (int i = 0; i < scanCount; i++) {
        strncpy(scanResults[i].ssid, WiFi.SSID(i).c_str(), 32);
        scanResults[i].ssid[32] = '\0';
        scanResults[i].rssi = WiFi.RSSI(i);
        scanResults[i].open = (WiFi.encryptionType(i) == WIFI_AUTH_OPEN);
    }

    WiFi.scanDelete();
    return scanCount;
}

WiFiNetwork* WiFiManager::getScanResults() {
    return scanResults;
}

int WiFiManager::getScanCount() {
    return scanCount;
}

bool WiFiManager::connect(const char* ssid, const char* password) {
    WiFi.disconnect();
    delay(100);

    if (password && strlen(password) > 0) {
        WiFi.begin(ssid, password);
    } else {
        WiFi.begin(ssid);
    }

    // Wait for connection (timeout 10 seconds)
    int timeout = 100;  // 100 * 100ms = 10s
    while (WiFi.status() != WL_CONNECTED && timeout > 0) {
        delay(100);
        timeout--;
    }

    if (WiFi.status() == WL_CONNECTED) {
        strncpy(currentSSID, ssid, sizeof(currentSSID) - 1);
        return true;
    }

    return false;
}

void WiFiManager::disconnect() {
    WiFi.disconnect();
    currentSSID[0] = '\0';
}

bool WiFiManager::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

bool WiFiManager::autoConnect() {
    for (int i = 0; i < savedCount; i++) {
        if (connect(savedSSIDs[i], savedPasswords[i])) {
            return true;
        }
    }
    return false;
}

const char* WiFiManager::getSSID() {
    if (isConnected()) {
        return currentSSID;
    }
    return "Not connected";
}

IPAddress WiFiManager::getIP() {
    return WiFi.localIP();
}

int32_t WiFiManager::getRSSI() {
    return WiFi.RSSI();
}

void WiFiManager::saveNetwork(const char* ssid, const char* password) {
    // Check if already saved
    for (int i = 0; i < savedCount; i++) {
        if (strcmp(savedSSIDs[i], ssid) == 0) {
            // Update password
            strncpy(savedPasswords[i], password, sizeof(savedPasswords[i]) - 1);
            saveSavedNetworks();
            return;
        }
    }

    // Add new
    if (savedCount < MAX_SAVED_NETWORKS) {
        strncpy(savedSSIDs[savedCount], ssid, sizeof(savedSSIDs[savedCount]) - 1);
        strncpy(savedPasswords[savedCount], password, sizeof(savedPasswords[savedCount]) - 1);
        savedCount++;
        saveSavedNetworks();
    } else {
        // Replace oldest (index 0)
        for (int i = 0; i < MAX_SAVED_NETWORKS - 1; i++) {
            strcpy(savedSSIDs[i], savedSSIDs[i + 1]);
            strcpy(savedPasswords[i], savedPasswords[i + 1]);
        }
        strncpy(savedSSIDs[MAX_SAVED_NETWORKS - 1], ssid, 32);
        strncpy(savedPasswords[MAX_SAVED_NETWORKS - 1], password, 64);
        saveSavedNetworks();
    }
}

void WiFiManager::forgetNetwork(int index) {
    if (index < 0 || index >= savedCount) return;

    for (int i = index; i < savedCount - 1; i++) {
        strcpy(savedSSIDs[i], savedSSIDs[i + 1]);
        strcpy(savedPasswords[i], savedPasswords[i + 1]);
    }
    savedCount--;
    saveSavedNetworks();
}

int WiFiManager::getSavedCount() {
    return savedCount;
}

const char* WiFiManager::getSavedSSID(int index) {
    if (index < 0 || index >= savedCount) return "";
    return savedSSIDs[index];
}

String WiFiManager::httpGet(const char* url) {
    if (!isConnected()) return "";

    HTTPClient http;
    http.begin(url);
    http.setTimeout(10000);

    int httpCode = http.GET();
    String payload = "";

    if (httpCode == HTTP_CODE_OK) {
        payload = http.getString();
    }

    http.end();
    return payload;
}
