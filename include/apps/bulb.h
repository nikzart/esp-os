#ifndef BULB_H
#define BULB_H

#include "app.h"
#include <Preferences.h>

// Preset structure for saved lighting settings
struct BulbPreset {
    char name[16];
    int hue;        // 0-360
    int saturation; // 0-100
    int brightness; // 0-100
    int colorTemp;  // 153-500 (mireds)
    bool valid;     // Whether preset has been saved
};

class BulbApp : public App {
public:
    void init() override;
    void update() override;
    void render() override;
    void onButton(uint8_t btn, bool pressed) override;
    const char* getName() override { return "Bulb"; }
    const uint8_t* getIcon() override;

private:
    // Screen modes
    enum class Mode {
        MAIN,
        BRIGHTNESS,
        COLOR,
        CT,
        PRESETS,
        SETTINGS
    };

    Mode mode = Mode::MAIN;
    int menuIndex = 0;      // Menu selection on main screen
    int colorIndex = 0;     // 0=hue, 1=saturation on color screen
    int presetIndex = 0;    // Selected preset

    // Bulb state
    char bulbIP[32] = "";
    bool power = false;
    int brightness = 100;
    int hue = 0;
    int saturation = 0;
    int colorTemp = 326;    // Mid-range (warm white)

    // UI state
    bool loading = false;
    bool hasConnection = false;
    char errorMsg[32] = "";
    unsigned long lastFetch = 0;
    bool editingIP = false;
    char ipBuffer[32] = "";

    // Presets
    static const int PRESET_COUNT = 4;
    BulbPreset presets[PRESET_COUNT];

    // Render methods
    void renderMain();
    void renderBrightness();
    void renderColor();
    void renderCT();
    void renderPresets();
    void renderSettings();

    // HTTP/Tasmota methods
    void sendCommand(const char* cmd);
    void fetchState();
    void setPower(bool on);
    void togglePower();
    void applyBrightness();
    void applyColor();
    void applyColorTemp();

    // Preset methods
    void loadPreset(int idx);
    void savePreset(int idx);

    // Storage methods
    void loadSettings();
    void saveSettings();
    void loadPresets();
    void savePresets();

    // Helper
    const char* getColorName();
    int ctToKelvin(int ct);
};

#endif
