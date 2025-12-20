#ifndef WEATHER_H
#define WEATHER_H

#include "app.h"

class WeatherApp : public App {
public:
    void init() override;
    void update() override;
    void render() override;
    void onButton(uint8_t btn, bool pressed) override;
    const char* getName() override { return "Weather"; }
    const uint8_t* getIcon() override;

private:
    bool loading = false;
    bool hasData = false;
    bool searching = false;
    char city[32] = "Kollam";
    char searchBuffer[32] = "";
    float temp = 0;
    int humidity = 0;
    char description[32] = "";
    char errorMsg[32] = "";
    unsigned long lastFetch = 0;

    void fetchWeather();
    void loadCity();
    void saveCity();
};

#endif
