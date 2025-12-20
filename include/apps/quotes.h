#ifndef QUOTES_H
#define QUOTES_H

#include "app.h"

class QuotesApp : public App {
public:
    void init() override;
    void update() override;
    void render() override;
    void onButton(uint8_t btn, bool pressed) override;
    const char* getName() override { return "Quotes"; }
    const uint8_t* getIcon() override;

private:
    bool loading = false;
    bool hasData = false;
    char errorMsg[32] = "";
    char quote[200] = "";
    char author[48] = "";

    void fetchQuote();
};

#endif
