#ifndef PONG_H
#define PONG_H

#include "app.h"

class PongApp : public App {
public:
    void init() override;
    void update() override;
    void render() override;
    void onButton(uint8_t btn, bool pressed) override;
    const char* getName() override { return "Pong"; }
    const uint8_t* getIcon() override;

private:
    enum class State {
        MENU,
        PLAYING,
        GAME_OVER
    };

    State state = State::MENU;

    // Paddle dimensions
    static const int PADDLE_W = 3;
    static const int PADDLE_H = 12;
    static const int BALL_SIZE = 3;

    // Player paddle (left)
    int playerY = 26;
    int playerScore = 0;

    // AI paddle (right)
    int aiY = 26;
    int aiScore = 0;
    float aiSpeed = 1.5;

    // Ball
    float ballX = 64;
    float ballY = 32;
    float ballVX = 2;
    float ballVY = 1;

    // Game settings
    int winScore = 5;
    unsigned long lastUpdate = 0;
    bool playerWon = false;

    void startGame();
    void resetBall(bool towardsPlayer);
    void updateAI();
};

#endif
