#ifndef CRYPTO_H
#define CRYPTO_H

#include "app.h"

class CryptoApp : public App {
public:
    void init() override;
    void update() override;
    void render() override;
    void onButton(uint8_t btn, bool pressed) override;
    const char* getName() override { return "Crypto"; }
    const uint8_t* getIcon() override;

private:
    bool loading = false;
    bool hasData = false;
    char errorMsg[32] = "";
    unsigned long lastFetch = 0;

    float btcPrice = 0;
    float ethPrice = 0;
    float solPrice = 0;
    float btcChange = 0;
    float ethChange = 0;
    float solChange = 0;

    void fetchPrices();
};

#endif
