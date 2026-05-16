#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <Preferences.h>

#define MAX_SAVED_NETWORKS 3
#define MAX_SCAN_RESULTS 10

struct WiFiNetwork {
    char ssid[33];
    int32_t rssi;
    bool open;
};

enum class WiFiConnectState : uint8_t {
    IDLE,
    CONNECTING,
    CONNECTED,
    FAILED_ALL
};

class WiFiManager {
public:
    static void init();

    // Scan for networks
    static int scan();
    static WiFiNetwork* getScanResults();
    static int getScanCount();

    // Connect to network (blocking, used by Settings UI)
    static bool connect(const char* ssid, const char* password);
    static void disconnect();
    static bool isConnected();

    // Auto-connect to saved networks (non-blocking; progresses via update())
    static bool autoConnect();

    // Drive the async connect state machine; call from main loop()
    static void update();
    static WiFiConnectState getConnectState();
    static int getConnectingIndex();

    // Get current connection info
    static const char* getSSID();
    static IPAddress getIP();
    static int32_t getRSSI();

    // Saved networks (NVS)
    static void saveNetwork(const char* ssid, const char* password);
    static void forgetNetwork(int index);
    static int getSavedCount();
    static const char* getSavedSSID(int index);

    // HTTP helper for API calls
    static String httpGet(const char* url);

private:
    static Preferences prefs;
    static WiFiNetwork scanResults[MAX_SCAN_RESULTS];
    static int scanCount;
    static char savedSSIDs[MAX_SAVED_NETWORKS][33];
    static char savedPasswords[MAX_SAVED_NETWORKS][65];
    static int savedCount;
    static char currentSSID[33];

    static WiFiConnectState connectState;
    static int connectingIndex;
    static unsigned long attemptStart;

    static void loadSavedNetworks();
    static void saveSavedNetworks();
    static void beginAttempt(int index);
};

#endif
