#ifndef TIMER_H
#define TIMER_H

#include "app.h"

class TimerApp : public App {
public:
    void init() override;
    void update() override;
    void render() override;
    void onButton(uint8_t btn, bool pressed) override;
    const char* getName() override { return "Timer"; }
    const uint8_t* getIcon() override;

private:
    enum class Mode { SELECT, COUNTDOWN_SETUP, COUNTDOWN, STOPWATCH };
    enum class State { STOPPED, RUNNING, PAUSED, FINISHED };

    Mode mode = Mode::SELECT;
    State state = State::STOPPED;
    int selectIndex = 0;  // 0=Countdown, 1=Stopwatch

    // Timer values (in milliseconds)
    unsigned long targetTime = 0;      // Countdown target
    unsigned long elapsedTime = 0;     // Elapsed time
    unsigned long startMillis = 0;     // When timer started
    unsigned long pausedTime = 0;      // Time accumulated before pause

    // Countdown setup
    int setHours = 0;
    int setMinutes = 5;
    int setSeconds = 0;
    int setupIndex = 0;  // 0=hours, 1=minutes, 2=seconds

    void renderModeSelect();
    void renderCountdownSetup();
    void renderCountdown();
    void renderStopwatch();
    void formatTime(unsigned long ms, char* buf, bool showHours = true);
    void playAlarm();
};

#endif
