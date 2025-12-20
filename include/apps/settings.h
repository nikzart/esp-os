#ifndef SETTINGS_H
#define SETTINGS_H

#include "app.h"

class SettingsApp : public App {
public:
    void init() override;
    void update() override;
    void render() override;
    void onButton(uint8_t btn, bool pressed) override;
    const char* getName() override { return "Settings"; }
    const uint8_t* getIcon() override;

private:
    enum class Mode {
        MENU,
        WIFI_LIST,
        WIFI_SCAN,
        WIFI_PASSWORD,
        API_KEYS,
        EDIT_API_KEY,
        BRIGHTNESS,
        SLEEP_TIMEOUT
    };

    Mode mode = Mode::MENU;
    int menuIndex = 0;
    int wifiIndex = 0;
    bool scanning = false;

    char weatherApiKey[48] = {0};
    char newsApiKey[48] = {0};
    char selectedSSID[33] = {0};

    int apiKeyIndex = 0;
    uint8_t brightness = 255;
    int sleepTimeoutIndex = 0;

    void loadSettings();
    void saveSettings();
    void renderMenu();
    void renderWifiList();
    void renderApiKeys();
    void renderBrightness();
    void renderSleepTimeout();
};

#endif
