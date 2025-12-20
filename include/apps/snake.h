#ifndef SNAKE_H
#define SNAKE_H

#include "app.h"

#define SNAKE_GRID_W 32
#define SNAKE_GRID_H 16
#define SNAKE_MAX_LEN 64

class SnakeApp : public App {
public:
    void init() override;
    void update() override;
    void render() override;
    void onButton(uint8_t btn, bool pressed) override;
    const char* getName() override { return "Snake"; }
    const uint8_t* getIcon() override;

private:
    enum class State {
        MENU,
        PLAYING,
        GAME_OVER
    };

    State state = State::MENU;

    // Snake body (circular buffer)
    int8_t snakeX[SNAKE_MAX_LEN];
    int8_t snakeY[SNAKE_MAX_LEN];
    int snakeLen = 3;
    int snakeHead = 0;

    // Direction: 0=right, 1=down, 2=left, 3=up
    int direction = 0;
    int nextDirection = 0;

    // Food
    int8_t foodX = 0;
    int8_t foodY = 0;

    // Game state
    int score = 0;
    int highScore = 0;
    unsigned long lastMove = 0;
    int speed = 150;  // ms per move

    void startGame();
    void moveSnake();
    void spawnFood();
    bool checkCollision(int x, int y);
    void loadHighScore();
    void saveHighScore();
};

#endif
