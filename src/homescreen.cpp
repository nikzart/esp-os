#include "homescreen.h"
#include "ui.h"
#include "config.h"
#include "wifi_manager.h"
#include <WiFi.h>
#include <time.h>
#include <ArduinoJson.h>

extern U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2;

// Time data
static bool timeSync = false;
static unsigned long lastTimeSync = 0;
static const unsigned long TIME_SYNC_INTERVAL = 3600000; // 1 hour

// Weather data
static char weatherTemp[8] = "--";
static char weatherDesc[16] = "";
static unsigned long lastWeatherFetch = 0;
static const unsigned long WEATHER_FETCH_INTERVAL = 1800000; // 30 minutes

// Days of week
static const char* daysOfWeek[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
static const char* months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", 
                                "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

void Homescreen::init() {
    // Try to sync time if WiFi connected
    if (WiFiManager::isConnected()) {
        syncTime();
        fetchWeather();
    }
}

void Homescreen::syncTime() {
    if (!WiFiManager::isConnected()) return;
    
    // Configure NTP
    configTime(19800, 0, "pool.ntp.org", "time.nist.gov"); // IST offset (5:30 = 19800 seconds)
    
    // Wait for time sync (max 5 seconds)
    struct tm timeinfo;
    int retry = 0;
    while (!getLocalTime(&timeinfo) && retry < 10) {
        delay(500);
        retry++;
    }
    
    if (retry < 10) {
        timeSync = true;
        lastTimeSync = millis();
        Serial.println("Time synced via NTP");
    }
}

void Homescreen::fetchWeather() {
    if (!WiFiManager::isConnected()) return;
    
    char url[200];
    snprintf(url, sizeof(url), 
        "http://api.openweathermap.org/data/2.5/weather?q=%s&appid=%s&units=metric",
        DEFAULT_CITY, OPENWEATHER_API_KEY);
    
    String response = WiFiManager::httpGet(url);
    
    if (response.length() > 0) {
        StaticJsonDocument<1024> doc;
        DeserializationError error = deserializeJson(doc, response);
        
        if (!error) {
            float temp = doc["main"]["temp"];
            const char* desc = doc["weather"][0]["main"];
            
            snprintf(weatherTemp, sizeof(weatherTemp), "%.0f", temp);
            strncpy(weatherDesc, desc, sizeof(weatherDesc) - 1);
            lastWeatherFetch = millis();
            
            Serial.printf("Weather: %s°C %s\n", weatherTemp, weatherDesc);
        }
    }
}

void Homescreen::update() {
    unsigned long now = millis();
    
    // Re-sync time periodically
    if (timeSync && (now - lastTimeSync > TIME_SYNC_INTERVAL)) {
        if (WiFiManager::isConnected()) {
            syncTime();
        }
    }
    
    // Re-fetch weather periodically
    if (now - lastWeatherFetch > WEATHER_FETCH_INTERVAL) {
        if (WiFiManager::isConnected()) {
            fetchWeather();
        }
    }
}

void Homescreen::render() {
    UI::clear();
    
    struct tm timeinfo;
    bool hasTime = getLocalTime(&timeinfo);
    
    // Large time display
    u8g2.setFont(u8g2_font_logisoso22_tn);
    if (hasTime) {
        char timeStr[8];
        snprintf(timeStr, sizeof(timeStr), "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
        int w = u8g2.getStrWidth(timeStr);
        u8g2.drawStr((128 - w) / 2, 24, timeStr);
    } else {
        u8g2.drawStr(38, 24, "--:--");
    }
    
    // Date
    u8g2.setFont(u8g2_font_5x7_tf);
    if (hasTime) {
        char dateStr[24];
        snprintf(dateStr, sizeof(dateStr), "%s, %s %d", 
                 daysOfWeek[timeinfo.tm_wday],
                 months[timeinfo.tm_mon],
                 timeinfo.tm_mday);
        int w = u8g2.getStrWidth(dateStr);
        u8g2.drawStr((128 - w) / 2, 34, dateStr);
    }
    
    // Divider line
    u8g2.drawHLine(10, 38, 108);
    
    // WiFi status
    if (WiFiManager::isConnected()) {
        int rssi = WiFiManager::getRSSI();
        const char* bars = rssi > -50 ? "***" : (rssi > -70 ? "**" : "*");
        char wifiStr[32];
        snprintf(wifiStr, sizeof(wifiStr), "WiFi: %s %s", WiFiManager::getSSID(), bars);
        // Truncate if too long
        if (strlen(wifiStr) > 24) {
            wifiStr[24] = '\0';
        }
        u8g2.drawStr(4, 48, wifiStr);
    } else {
        u8g2.drawStr(4, 48, "WiFi: Not connected");
    }
    
    // Weather
    if (strlen(weatherDesc) > 0) {
        char weatherStr[24];
        snprintf(weatherStr, sizeof(weatherStr), "%s C  %s", weatherTemp, weatherDesc);
        u8g2.drawStr(4, 57, weatherStr);
    } else {
        u8g2.drawStr(4, 57, "Weather: --");
    }
    
    // Bottom hint
    UI::setSmallFont();
    u8g2.drawStr(80, 64, "[A] Apps");
    UI::setNormalFont();
    
    UI::flush();
}

bool Homescreen::onButton(uint8_t btn, bool pressed) {
    if (!pressed) return false;
    
    // Any button goes to launcher
    if (btn == BTN_A || btn == BTN_UP || btn == BTN_DOWN || 
        btn == BTN_LEFT || btn == BTN_RIGHT || btn == BTN_D) {
        UI::beep();
        return true; // Go to launcher
    }
    
    // C button refreshes data
    if (btn == BTN_C) {
        UI::beep();
        if (WiFiManager::isConnected()) {
            syncTime();
            fetchWeather();
        }
    }
    
    return false;
}
