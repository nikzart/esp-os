#ifndef LAUNCHER_H
#define LAUNCHER_H

#include "app.h"

class LauncherApp : public App {
public:
    void init() override;
    void update() override;
    void render() override;
    void onButton(uint8_t btn, bool pressed) override;
    const char* getName() override { return "Launcher"; }
    const uint8_t* getIcon() override { return nullptr; }

    // Get selected app index
    int getSelectedApp();

    // Set app list
    void setApps(App** apps, int count);

private:
    App** appList = nullptr;
    int appCount = 0;
    int selectedIndex = 0;
    int scrollOffset = 0;

    // Grid layout: 4 columns, 2 visible rows
    static const int COLS = 4;
    static const int VISIBLE_ROWS = 2;
    static const int ICON_SIZE = 16;
    static const int CELL_WIDTH = 32;
    static const int CELL_HEIGHT = 24;

    void drawAppIcon(int index, int gridX, int gridY, bool selected);
};

#endif
