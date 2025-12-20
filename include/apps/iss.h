#ifndef ISS_H
#define ISS_H

#include "app.h"

class ISSApp : public App {
public:
    void init() override;
    void update() override;
    void render() override;
    void onButton(uint8_t btn, bool pressed) override;
    const char* getName() override { return "ISS"; }
    const uint8_t* getIcon() override;

private:
    bool loading = false;
    bool hasData = false;
    char errorMsg[32] = "";
    unsigned long lastFetch = 0;

    float latitude = 0;
    float longitude = 0;
    int astronautCount = 0;
    char astronauts[6][32];  // First 6 astronauts

    void fetchISS();
    void fetchAstronauts();
};

#endif
