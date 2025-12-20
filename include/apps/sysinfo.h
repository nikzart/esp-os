#ifndef SYSINFO_H
#define SYSINFO_H

#include "app.h"

class SysInfoApp : public App {
public:
    void init() override;
    void update() override;
    void render() override;
    void onButton(uint8_t btn, bool pressed) override;
    const char* getName() override { return "System"; }
    const uint8_t* getIcon() override;

private:
    enum class Page {
        INFO,
        BUTTON_TEST
    };

    Page currentPage = Page::INFO;

    void renderInfo();
    void renderButtonTest();
};

#endif
