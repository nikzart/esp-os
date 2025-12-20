#ifndef NEWS_H
#define NEWS_H

#include "app.h"

#define MAX_HEADLINES 5

class NewsApp : public App {
public:
    void init() override;
    void update() override;
    void render() override;
    void onButton(uint8_t btn, bool pressed) override;
    const char* getName() override { return "News"; }
    const uint8_t* getIcon() override;

private:
    bool loading = false;
    bool hasData = false;
    char errorMsg[32] = "";
    unsigned long lastFetch = 0;

    char headlines[MAX_HEADLINES][80];
    char sources[MAX_HEADLINES][24];
    int headlineCount = 0;
    int currentIndex = 0;

    void fetchNews();
};

#endif
