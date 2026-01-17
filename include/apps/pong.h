#ifndef PONG_H
#define PONG_H

#include "app.h"
#include <WebSocketsServer.h>

class PongApp : public App {
public:
    void init() override;
    void update() override;
    void render() override;
    void onButton(uint8_t btn, bool pressed) override;
    void onClose() override;
    const char* getName() override { return "Pong"; }
    const uint8_t* getIcon() override;

private:
    enum class State {
        MENU,
        MODE_SELECT,
        WAITING_P2,
        PLAYING,
        GAME_OVER
    };

    enum class GameMode {
        VS_AI,
        VS_PLAYER
    };

    State state = State::MENU;
    GameMode gameMode = GameMode::VS_AI;

    // Paddle dimensions
    static const int PADDLE_W = 3;
    static const int PADDLE_H = 12;
    static const int BALL_SIZE = 3;

    // Player paddle (left)
    int playerY = 26;
    int playerScore = 0;

    // AI/Player2 paddle (right)
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

    // Mode selection
    int modeSelectIndex = 0;

    // WebSocket for PvP
    WebSocketsServer* wsServer = nullptr;
    bool player2Connected = false;
    int player2ClientNum = -1;
    unsigned long lastP2Update = 0;
    bool player2Ready = false;

    void startGame();
    void resetBall(bool towardsPlayer);
    void updateAI();

    // WebSocket methods
    void startWebSocketServer();
    void stopWebSocketServer();
    void sendGameState();
    void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length);
    static void wsEventHandler(uint8_t num, WStype_t type, uint8_t* payload, size_t length);
};

#endif
