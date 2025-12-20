#ifndef HOMESCREEN_H
#define HOMESCREEN_H

#include <Arduino.h>

namespace Homescreen {
    // Initialize homescreen (call after WiFi init)
    void init();
    
    // Update data (weather, time sync)
    void update();
    
    // Render the homescreen
    void render();
    
    // Handle button input - returns true if should go to launcher
    bool onButton(uint8_t btn, bool pressed);
    
    // Sync time with NTP server
    void syncTime();

    // Fetch weather data
    void fetchWeather();

    // Load time settings from NVS
    void loadTimeSettings();
}

#endif
