#ifndef FACTS_H
#define FACTS_H

#include "app.h"

class FactsApp : public App {
public:
    void init() override;
    void update() override;
    void render() override;
    void onButton(uint8_t btn, bool pressed) override;
    const char* getName() override { return "Facts"; }
    const uint8_t* getIcon() override;

private:
    bool loading = false;
    bool hasData = false;
    char errorMsg[32] = "";
    char fact[256] = "";

    void fetchFact();
};

#endif
