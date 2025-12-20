#include "apps/timer.h"
#include "ui.h"
#include "icons.h"
#include "input.h"

extern U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2;

void TimerApp::init() {
    mode = Mode::SELECT;
    state = State::STOPPED;
    selectIndex = 0;
    elapsedTime = 0;
    pausedTime = 0;
    setHours = 0;
    setMinutes = 5;
    setSeconds = 0;
    setupIndex = 1;  // Start on minutes
}

void TimerApp::update() {
    if (state == State::RUNNING) {
        unsigned long now = millis();
        elapsedTime = pausedTime + (now - startMillis);

        // Check countdown completion
        if (mode == Mode::COUNTDOWN && elapsedTime >= targetTime) {
            elapsedTime = targetTime;
            state = State::FINISHED;
            playAlarm();
        }
    }
}

void TimerApp::render() {
    UI::clear();

    switch (mode) {
        case Mode::SELECT:
            renderModeSelect();
            break;
        case Mode::COUNTDOWN_SETUP:
            renderCountdownSetup();
            break;
        case Mode::COUNTDOWN:
            renderCountdown();
            break;
        case Mode::STOPWATCH:
            renderStopwatch();
            break;
    }

    UI::flush();
}

void TimerApp::renderModeSelect() {
    UI::drawTitleBar("Timer");

    const char* items[] = {"Countdown", "Stopwatch"};
    for (int i = 0; i < 2; i++) {
        UI::drawMenuItem(28 + i * 12, items[i], i == selectIndex);
    }

    UI::drawStatusBar("A:Select", "B:Back");
}

void TimerApp::renderCountdownSetup() {
    UI::drawTitleBar("Set Timer");

    u8g2.setFont(u8g2_font_logisoso16_tn);

    // Calculate widths for proper positioning
    int digitWidth = u8g2.getStrWidth("00");
    int colonWidth = u8g2.getStrWidth(":");
    int totalWidth = digitWidth * 3 + colonWidth * 2;
    int startX = (128 - totalWidth) / 2;

    // Draw each component with proper spacing
    char buf[4];
    int x = startX;

    // Hours
    snprintf(buf, sizeof(buf), "%02d", setHours);
    int hoursX = x;
    u8g2.drawStr(x, 38, buf);
    x += digitWidth;

    // Colon 1
    u8g2.drawStr(x, 38, ":");
    x += colonWidth;

    // Minutes
    snprintf(buf, sizeof(buf), "%02d", setMinutes);
    int minutesX = x;
    u8g2.drawStr(x, 38, buf);
    x += digitWidth;

    // Colon 2
    u8g2.drawStr(x, 38, ":");
    x += colonWidth;

    // Seconds
    snprintf(buf, sizeof(buf), "%02d", setSeconds);
    int secondsX = x;
    u8g2.drawStr(x, 38, buf);

    // Draw selector box around active field
    int positions[] = {hoursX, minutesX, secondsX};
    u8g2.drawFrame(positions[setupIndex] - 2, 22, digitWidth + 4, 20);

    // Labels centered under each field
    UI::setSmallFont();
    const char* labels[] = {"hr", "min", "sec"};
    for (int i = 0; i < 3; i++) {
        int labelW = u8g2.getStrWidth(labels[i]);
        u8g2.drawStr(positions[i] + (digitWidth - labelW) / 2, 52, labels[i]);
    }

    UI::setNormalFont();
    UI::drawStatusBar("U/D:Adj A:Start", "B:Back");
}

void TimerApp::renderCountdown() {
    UI::drawTitleBar("Countdown");

    unsigned long remaining = 0;
    if (elapsedTime < targetTime) {
        remaining = targetTime - elapsedTime;
    }

    // Large centered time display
    char timeStr[12];
    formatTime(remaining, timeStr, remaining >= 3600000);

    u8g2.setFont(u8g2_font_logisoso22_tn);
    int w = u8g2.getStrWidth(timeStr);
    u8g2.drawStr((128 - w) / 2, 36, timeStr);

    // Progress bar - shows remaining time (depletes as countdown progresses)
    int progress = 0;
    if (targetTime > 0) {
        progress = (int)((remaining * 100) / targetTime);
        if (progress > 100) progress = 100;
    }
    UI::drawProgressBar(14, 44, 100, 6, progress);

    // Status text - only show when paused or finished
    UI::setSmallFont();
    const char* statusText = "";
    if (state == State::FINISHED) {
        statusText = "TIME'S UP!";
    } else if (state == State::PAUSED) {
        statusText = "PAUSED";
    }
    if (strlen(statusText) > 0) {
        int sw = u8g2.getStrWidth(statusText);
        u8g2.drawStr((128 - sw) / 2, 52, statusText);
    }

    UI::setNormalFont();

    if (state == State::FINISHED) {
        UI::drawStatusBar("A:Reset", "B:Back");
    } else if (state == State::PAUSED) {
        UI::drawStatusBar("A:Resume B:Reset", "D:Back");
    } else if (state == State::RUNNING) {
        UI::drawStatusBar("A:Pause", "B:Reset");
    } else {
        UI::drawStatusBar("A:Start", "B:Back");
    }
}

void TimerApp::renderStopwatch() {
    UI::drawTitleBar("Stopwatch");

    // Large time display
    char timeStr[12];
    formatTime(elapsedTime, timeStr, true);

    u8g2.setFont(u8g2_font_logisoso22_tn);
    int w = u8g2.getStrWidth(timeStr);
    u8g2.drawStr((128 - w) / 2, 38, timeStr);

    UI::setNormalFont();

    if (state == State::PAUSED) {
        UI::drawStatusBar("A:Resume B:Reset", "D:Back");
    } else if (state == State::RUNNING) {
        UI::drawStatusBar("A:Pause", "B:Reset");
    } else {
        UI::drawStatusBar("A:Start", "B:Back");
    }
}

void TimerApp::formatTime(unsigned long ms, char* buf, bool showHours) {
    unsigned long totalSec = ms / 1000;
    int hours = totalSec / 3600;
    int minutes = (totalSec % 3600) / 60;
    int seconds = totalSec % 60;

    if (showHours && hours > 0) {
        snprintf(buf, 12, "%d:%02d:%02d", hours, minutes, seconds);
    } else {
        snprintf(buf, 12, "%02d:%02d", minutes, seconds);
    }
}

void TimerApp::playAlarm() {
    // Play alarm beeps
    for (int i = 0; i < 5; i++) {
        UI::beep(2000, 100);
        delay(150);
    }
}

void TimerApp::onButton(uint8_t btn, bool pressed) {
    if (!pressed) return;

    UI::beep();

    switch (mode) {
        case Mode::SELECT:
            if (btn == BTN_UP && selectIndex > 0) selectIndex--;
            else if (btn == BTN_DOWN && selectIndex < 1) selectIndex++;
            else if (btn == BTN_A) {
                if (selectIndex == 0) {
                    mode = Mode::COUNTDOWN_SETUP;
                    setupIndex = 1;  // Start on minutes
                } else {
                    mode = Mode::STOPWATCH;
                    state = State::STOPPED;
                    elapsedTime = 0;
                    pausedTime = 0;
                }
            } else if (btn == BTN_B || btn == BTN_D) {
                wantsToExit = true;
            }
            break;

        case Mode::COUNTDOWN_SETUP:
            if (btn == BTN_LEFT && setupIndex > 0) setupIndex--;
            else if (btn == BTN_RIGHT && setupIndex < 2) setupIndex++;
            else if (btn == BTN_UP) {
                if (setupIndex == 0) setHours = (setHours + 1) % 24;
                else if (setupIndex == 1) setMinutes = (setMinutes + 1) % 60;
                else setSeconds = (setSeconds + 1) % 60;
            } else if (btn == BTN_DOWN) {
                if (setupIndex == 0) setHours = (setHours + 23) % 24;
                else if (setupIndex == 1) setMinutes = (setMinutes + 59) % 60;
                else setSeconds = (setSeconds + 59) % 60;
            } else if (btn == BTN_A) {
                // Start countdown
                targetTime = (unsigned long)(setHours * 3600 + setMinutes * 60 + setSeconds) * 1000;
                if (targetTime > 0) {
                    mode = Mode::COUNTDOWN;
                    state = State::RUNNING;
                    elapsedTime = 0;
                    pausedTime = 0;
                    startMillis = millis();
                }
            } else if (btn == BTN_B) {
                mode = Mode::SELECT;
            }
            break;

        case Mode::COUNTDOWN:
            if (btn == BTN_A) {
                if (state == State::RUNNING) {
                    // Pause
                    state = State::PAUSED;
                    pausedTime = elapsedTime;
                } else if (state == State::PAUSED) {
                    // Resume
                    state = State::RUNNING;
                    startMillis = millis();
                } else if (state == State::FINISHED) {
                    // Reset
                    state = State::STOPPED;
                    elapsedTime = 0;
                    pausedTime = 0;
                    mode = Mode::COUNTDOWN_SETUP;
                }
            } else if (btn == BTN_B) {
                if (state == State::RUNNING || state == State::PAUSED) {
                    // Reset
                    state = State::STOPPED;
                    elapsedTime = 0;
                    pausedTime = 0;
                } else {
                    mode = Mode::SELECT;
                }
            } else if (btn == BTN_D) {
                state = State::STOPPED;
                elapsedTime = 0;
                pausedTime = 0;
                mode = Mode::SELECT;
            }
            break;

        case Mode::STOPWATCH:
            if (btn == BTN_A) {
                if (state == State::RUNNING) {
                    // Pause
                    state = State::PAUSED;
                    pausedTime = elapsedTime;
                } else if (state == State::PAUSED || state == State::STOPPED) {
                    // Start/Resume
                    state = State::RUNNING;
                    startMillis = millis();
                }
            } else if (btn == BTN_B) {
                if (state == State::RUNNING || state == State::PAUSED) {
                    // Reset
                    state = State::STOPPED;
                    elapsedTime = 0;
                    pausedTime = 0;
                } else {
                    mode = Mode::SELECT;
                }
            } else if (btn == BTN_D) {
                state = State::STOPPED;
                elapsedTime = 0;
                pausedTime = 0;
                mode = Mode::SELECT;
            }
            break;
    }
}

const uint8_t* TimerApp::getIcon() {
    return icon_timer;
}
