#ifndef JOKES_H
#define JOKES_H

#include "app.h"

class JokesApp : public App {
public:
    void init() override;
    void update() override;
    void render() override;
    void onButton(uint8_t btn, bool pressed) override;
    const char* getName() override { return "Jokes"; }
    const uint8_t* getIcon() override;

private:
    bool loading = false;
    bool hasData = false;
    bool showPunchline = false;
    char errorMsg[32] = "";
    char setup[128] = "";
    char delivery[128] = "";
    bool isSingleJoke = false;

    void fetchJoke();
};

#endif
