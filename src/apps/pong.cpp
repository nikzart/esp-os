#include "apps/pong.h"
#include "ui.h"
#include "input.h"
#include "icons.h"
#include "wifi_manager.h"
#include <WebSocketsServer.h>
#include <WebServer.h>

// Static instance for WebSocket callback
static PongApp* pongInstance = nullptr;
static WebServer* httpServer = nullptr;

// Embedded HTML page for Player 2 (~2KB)
static const char PLAYER2_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width,initial-scale=1,user-scalable=no">
<title>Pong P2</title>
<style>
*{margin:0;padding:0;box-sizing:border-box}
body{background:#111;color:#fff;font-family:sans-serif;display:flex;flex-direction:column;align-items:center;height:100vh;padding:10px;overflow:hidden}
h1{font-size:18px;margin:5px 0}
#score{font-size:24px;margin:5px 0}
canvas{border:1px solid #444;background:#000;margin:10px 0}
#status{font-size:14px;color:#888;margin:5px 0}
.controls{display:flex;gap:20px;margin:10px 0}
.btn{width:80px;height:80px;border-radius:50%;border:none;font-size:32px;cursor:pointer;display:flex;align-items:center;justify-content:center;user-select:none;-webkit-user-select:none;touch-action:manipulation}
.btn-up{background:#2a5;color:#fff}
.btn-down{background:#25a;color:#fff}
.btn:active{opacity:0.7}
#ready{background:#e82;color:#fff;border:none;padding:15px 40px;font-size:18px;border-radius:8px;cursor:pointer;margin:10px 0;display:none}
#ready:active{opacity:0.7}
</style>
</head>
<body>
<h1>PONG - Player 2</h1>
<div id="score">0 - 0</div>
<canvas id="c" width="256" height="128"></canvas>
<div id="status">Connecting...</div>
<button id="ready">READY</button>
<div class="controls">
<button class="btn btn-up" id="up">&#9650;</button>
<button class="btn btn-down" id="down">&#9660;</button>
</div>
<script>
var ws,ctx=document.getElementById('c').getContext('2d');
var st={p1:26,p2:26,bx:64,by:32,s1:0,s2:0,state:0};
var connected=false,playing=false;
function draw(){
ctx.fillStyle='#000';ctx.fillRect(0,0,256,128);
ctx.fillStyle='#fff';
ctx.setLineDash([8,8]);ctx.beginPath();ctx.moveTo(128,0);ctx.lineTo(128,128);ctx.strokeStyle='#444';ctx.stroke();
ctx.fillRect(8,st.p1*2,6,24);
ctx.fillRect(242,st.p2*2,6,24);
ctx.fillRect(st.bx*2,st.by*2,6,6);
document.getElementById('score').textContent=st.s1+' - '+st.s2;
}
function connect(){
var h=location.hostname;
ws=new WebSocket('ws://'+h+':81/');
ws.onopen=function(){
connected=true;
document.getElementById('status').textContent='Connected - Press READY';
document.getElementById('ready').style.display='block';
};
ws.onclose=function(){
connected=false;playing=false;
document.getElementById('status').textContent='Disconnected';
document.getElementById('ready').style.display='none';
setTimeout(connect,2000);
};
ws.onmessage=function(e){
try{
var d=JSON.parse(e.data);
st.p1=d.p1;st.p2=d.p2;st.bx=d.bx;st.by=d.by;st.s1=d.s1;st.s2=d.s2;st.state=d.st;
if(d.st==3)playing=true;
if(d.st==4){
playing=false;
document.getElementById('status').textContent=(st.s2>st.s1)?'You Win!':'You Lose';
document.getElementById('ready').style.display='block';
document.getElementById('ready').textContent='REMATCH';
}else if(playing){
document.getElementById('status').textContent='Playing';
document.getElementById('ready').style.display='none';
}
draw();
}catch(ex){}
};
}
function send(c){if(ws&&ws.readyState==1)ws.send(c);}
var upInt,downInt;
document.getElementById('up').addEventListener('touchstart',function(e){e.preventDefault();send('U');upInt=setInterval(function(){send('U');},50);});
document.getElementById('up').addEventListener('touchend',function(){clearInterval(upInt);});
document.getElementById('up').addEventListener('mousedown',function(){send('U');upInt=setInterval(function(){send('U');},50);});
document.getElementById('up').addEventListener('mouseup',function(){clearInterval(upInt);});
document.getElementById('up').addEventListener('mouseleave',function(){clearInterval(upInt);});
document.getElementById('down').addEventListener('touchstart',function(e){e.preventDefault();send('D');downInt=setInterval(function(){send('D');},50);});
document.getElementById('down').addEventListener('touchend',function(){clearInterval(downInt);});
document.getElementById('down').addEventListener('mousedown',function(){send('D');downInt=setInterval(function(){send('D');},50);});
document.getElementById('down').addEventListener('mouseup',function(){clearInterval(downInt);});
document.getElementById('down').addEventListener('mouseleave',function(){clearInterval(downInt);});
document.getElementById('ready').addEventListener('click',function(){send('R');});
var upKey=false,downKey=false;
document.addEventListener('keydown',function(e){
if(e.key=='ArrowUp'||e.key=='w'||e.key=='W'){e.preventDefault();if(!upKey){upKey=true;send('U');upInt=setInterval(function(){send('U');},50);}}
if(e.key=='ArrowDown'||e.key=='s'||e.key=='S'){e.preventDefault();if(!downKey){downKey=true;send('D');downInt=setInterval(function(){send('D');},50);}}
if(e.key=='r'||e.key=='R'||e.key==' '||e.key=='Enter'){e.preventDefault();send('R');}
});
document.addEventListener('keyup',function(e){
if(e.key=='ArrowUp'||e.key=='w'||e.key=='W'){upKey=false;clearInterval(upInt);}
if(e.key=='ArrowDown'||e.key=='s'||e.key=='S'){downKey=false;clearInterval(downInt);}
});
draw();connect();
</script>
</body>
</html>
)rawliteral";

void PongApp::init() {
    state = State::MENU;
    pongInstance = this;
}

void PongApp::onClose() {
    stopWebSocketServer();
    pongInstance = nullptr;
}

void PongApp::startGame() {
    playerY = 26;
    aiY = 26;
    playerScore = 0;
    aiScore = 0;
    aiSpeed = 1.5;
    player2Ready = false;

    resetBall(random(0, 2) == 0);
    state = State::PLAYING;
    lastUpdate = millis();
}

void PongApp::resetBall(bool towardsPlayer) {
    ballX = 64;
    ballY = 32;

    float speed = 2.0;
    float angle = random(-45, 46) * PI / 180.0;

    ballVX = cos(angle) * speed * (towardsPlayer ? -1 : 1);
    ballVY = sin(angle) * speed;
}

void PongApp::updateAI() {
    // Simple AI: move towards ball
    float targetY = ballY - PADDLE_H / 2;

    // Add some prediction
    if (ballVX > 0) {
        // Ball coming towards AI
        float timeToReach = (120 - ballX) / ballVX;
        targetY = ballY + ballVY * timeToReach - PADDLE_H / 2;
    }

    // Move towards target
    if (aiY < targetY - 2) {
        aiY += aiSpeed;
    } else if (aiY > targetY + 2) {
        aiY -= aiSpeed;
    }

    // Clamp to screen
    if (aiY < 0) aiY = 0;
    if (aiY > 64 - PADDLE_H) aiY = 64 - PADDLE_H;
}

// WebSocket event handler (static wrapper)
void PongApp::wsEventHandler(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
    if (pongInstance) {
        pongInstance->onWebSocketEvent(num, type, payload, length);
    }
}

void PongApp::onWebSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
    switch (type) {
        case WStype_DISCONNECTED:
            if (num == player2ClientNum) {
                player2Connected = false;
                player2ClientNum = -1;
                player2Ready = false;
                // If playing, P1 wins by forfeit
                if (state == State::PLAYING) {
                    playerWon = true;
                    state = State::GAME_OVER;
                }
            }
            break;

        case WStype_CONNECTED:
            if (!player2Connected) {
                player2Connected = true;
                player2ClientNum = num;
                player2Ready = false;
            } else {
                // Reject additional connections
                wsServer->disconnect(num);
            }
            break;

        case WStype_TEXT:
            if (num == player2ClientNum && length > 0) {
                char cmd = (char)payload[0];
                if (cmd == 'U') {
                    // Move P2 paddle up
                    aiY -= 3;
                    if (aiY < 0) aiY = 0;
                } else if (cmd == 'D') {
                    // Move P2 paddle down
                    aiY += 3;
                    if (aiY > 64 - PADDLE_H) aiY = 64 - PADDLE_H;
                } else if (cmd == 'R') {
                    // P2 ready
                    if (state == State::WAITING_P2 || state == State::GAME_OVER) {
                        player2Ready = true;
                        if (state == State::WAITING_P2) {
                            startGame();
                        }
                    }
                }
            }
            break;

        default:
            break;
    }
}

// HTTP handler for serving Player 2 HTML page
static void handleRoot() {
    if (httpServer) {
        httpServer->send_P(200, "text/html", PLAYER2_HTML);
    }
}

void PongApp::startWebSocketServer() {
    if (wsServer) return;

    // Start HTTP server on port 80 for HTML page
    httpServer = new WebServer(80);
    httpServer->on("/", handleRoot);
    httpServer->begin();

    // Start WebSocket server on port 81
    wsServer = new WebSocketsServer(81);
    wsServer->begin();
    wsServer->onEvent(wsEventHandler);

    player2Connected = false;
    player2ClientNum = -1;
    player2Ready = false;
    lastP2Update = 0;
}

void PongApp::stopWebSocketServer() {
    if (httpServer) {
        httpServer->stop();
        delete httpServer;
        httpServer = nullptr;
    }
    if (wsServer) {
        wsServer->close();
        delete wsServer;
        wsServer = nullptr;
    }
    player2Connected = false;
    player2ClientNum = -1;
    player2Ready = false;
}

void PongApp::sendGameState() {
    if (!wsServer || !player2Connected) return;

    // Throttle to 30 FPS
    if (millis() - lastP2Update < 33) return;
    lastP2Update = millis();

    // State code: 2=waiting, 3=playing, 4=game_over
    int stateCode = (state == State::PLAYING) ? 3 :
                    (state == State::GAME_OVER) ? 4 : 2;

    char json[80];
    snprintf(json, sizeof(json),
        "{\"p1\":%d,\"p2\":%d,\"bx\":%d,\"by\":%d,\"s1\":%d,\"s2\":%d,\"st\":%d}",
        playerY, aiY, (int)ballX, (int)ballY, playerScore, aiScore, stateCode);

    wsServer->sendTXT(player2ClientNum, json);
}

void PongApp::update() {
    // Handle HTTP server
    if (httpServer) {
        httpServer->handleClient();
    }

    // Handle WebSocket server loop
    if (wsServer) {
        wsServer->loop();
    }

    // Send game state to P2 in PvP mode
    if (gameMode == GameMode::VS_PLAYER && (state == State::PLAYING || state == State::WAITING_P2 || state == State::GAME_OVER)) {
        sendGameState();
    }

    if (state != State::PLAYING) return;

    // Frame rate control (~60 FPS)
    if (millis() - lastUpdate < 16) return;
    lastUpdate = millis();

    // Player input
    if (Input::isPressed(BTN_UP)) {
        playerY -= 3;
        if (playerY < 0) playerY = 0;
    }
    if (Input::isPressed(BTN_DOWN)) {
        playerY += 3;
        if (playerY > 64 - PADDLE_H) playerY = 64 - PADDLE_H;
    }

    // Update AI (only in VS_AI mode)
    if (gameMode == GameMode::VS_AI) {
        updateAI();
    }

    // Move ball
    ballX += ballVX;
    ballY += ballVY;

    // Top/bottom wall bounce
    if (ballY <= 0 || ballY >= 64 - BALL_SIZE) {
        ballVY = -ballVY;
        ballY = constrain(ballY, 0, 64 - BALL_SIZE);
        UI::beep(1500, 20);
    }

    // Player paddle collision (left side)
    if (ballX <= PADDLE_W + 4 && ballX >= 4) {
        if (ballY + BALL_SIZE >= playerY && ballY <= playerY + PADDLE_H) {
            ballVX = abs(ballVX) * 1.05;  // Speed up slightly
            ballVY += (ballY - (playerY + PADDLE_H / 2)) * 0.1;
            ballX = PADDLE_W + 5;
            UI::beep(2000, 30);
        }
    }

    // AI/P2 paddle collision (right side)
    if (ballX >= 128 - PADDLE_W - 4 - BALL_SIZE && ballX <= 128 - 4 - BALL_SIZE) {
        if (ballY + BALL_SIZE >= aiY && ballY <= aiY + PADDLE_H) {
            ballVX = -abs(ballVX) * 1.05;
            ballVY += (ballY - (aiY + PADDLE_H / 2)) * 0.1;
            ballX = 128 - PADDLE_W - 5 - BALL_SIZE;
            UI::beep(2000, 30);
        }
    }

    // Scoring
    if (ballX < 0) {
        // AI/P2 scores
        aiScore++;
        UI::beep(500, 100);
        if (aiScore >= winScore) {
            playerWon = false;
            state = State::GAME_OVER;
        } else {
            resetBall(true);
        }
    } else if (ballX > 128) {
        // Player scores
        playerScore++;
        UI::beep(1000, 100);
        if (playerScore >= winScore) {
            playerWon = true;
            state = State::GAME_OVER;
        } else {
            resetBall(false);
        }
    }

    // Increase AI speed as game progresses (only in AI mode)
    if (gameMode == GameMode::VS_AI) {
        aiSpeed = 1.5 + (playerScore + aiScore) * 0.1;
    }
}

void PongApp::render() {
    UI::clear();

    switch (state) {
        case State::MENU:
            UI::drawTitleBar("Pong");
            UI::drawCentered(32, "Classic Pong Game");
            UI::drawStatusBar("A:Play", "B:Exit");
            break;

        case State::MODE_SELECT:
            UI::drawTitleBar("Game Mode");
            {
                const char* modes[] = {"vs AI", "vs Player"};
                for (int i = 0; i < 2; i++) {
                    int y = 24 + i * 14;
                    if (i == modeSelectIndex) {
                        u8g2.drawBox(10, y - 2, 108, 12);
                        u8g2.setDrawColor(0);
                        UI::drawCentered(y + 7, modes[i]);
                        u8g2.setDrawColor(1);
                    } else {
                        UI::drawCentered(y + 7, modes[i]);
                    }
                }
            }
            UI::drawStatusBar("A:Select", "B:Back");
            break;

        case State::WAITING_P2:
            UI::drawTitleBar("vs Player");
            {
                IPAddress ip = WiFiManager::getIP();
                char ipStr[24];
                snprintf(ipStr, sizeof(ipStr), "%d.%d.%d.%d",
                    ip[0], ip[1], ip[2], ip[3]);
                UI::drawCentered(24, "Open in browser:");
                UI::drawCentered(36, ipStr);

                if (player2Connected) {
                    UI::drawCentered(50, "P2 Connected!");
                } else {
                    // Animated waiting dots
                    int dots = (millis() / 500) % 4;
                    char wait[16] = "Waiting";
                    for (int i = 0; i < dots; i++) strcat(wait, ".");
                    UI::drawCentered(50, wait);
                }
            }
            UI::drawStatusBar("", "B:Cancel");
            break;

        case State::PLAYING:
            // Center line
            for (int y = 0; y < 64; y += 8) {
                u8g2.drawVLine(64, y, 4);
            }

            // Score
            {
                char buf[8];
                snprintf(buf, sizeof(buf), "%d", playerScore);
                u8g2.drawStr(50, 10, buf);
                snprintf(buf, sizeof(buf), "%d", aiScore);
                u8g2.drawStr(74, 10, buf);
            }

            // Player paddle (left)
            u8g2.drawBox(4, playerY, PADDLE_W, PADDLE_H);

            // AI/P2 paddle (right)
            u8g2.drawBox(128 - 4 - PADDLE_W, aiY, PADDLE_W, PADDLE_H);

            // Ball
            u8g2.drawBox((int)ballX, (int)ballY, BALL_SIZE, BALL_SIZE);
            break;

        case State::GAME_OVER:
            UI::drawTitleBar("Game Over");

            if (playerWon) {
                UI::drawCentered(28, gameMode == GameMode::VS_AI ? "You Win!" : "P1 Wins!");
            } else {
                UI::drawCentered(28, gameMode == GameMode::VS_AI ? "AI Wins!" : "P2 Wins!");
            }

            {
                char buf[16];
                snprintf(buf, sizeof(buf), "%d - %d", playerScore, aiScore);
                UI::drawCentered(42, buf);
            }

            UI::drawStatusBar("A:Again", "B:Exit");
            break;
    }

    UI::flush();
}

void PongApp::onButton(uint8_t btn, bool pressed) {
    if (!pressed) return;

    switch (state) {
        case State::MENU:
            if (btn == BTN_A) {
                state = State::MODE_SELECT;
                modeSelectIndex = 0;
                UI::beep();
            } else if (btn == BTN_B || btn == BTN_D) {
                wantsToExit = true;
            }
            break;

        case State::MODE_SELECT:
            if (btn == BTN_UP) {
                modeSelectIndex = 0;
                UI::beep();
            } else if (btn == BTN_DOWN) {
                modeSelectIndex = 1;
                UI::beep();
            } else if (btn == BTN_A) {
                if (modeSelectIndex == 0) {
                    // vs AI
                    gameMode = GameMode::VS_AI;
                    startGame();
                } else {
                    // vs Player - check WiFi
                    if (!WiFiManager::isConnected()) {
                        // Show warning briefly - can't do PvP without WiFi
                        // Just beep for now, user needs to connect WiFi first
                        UI::beep(200, 100);
                    } else {
                        gameMode = GameMode::VS_PLAYER;
                        startWebSocketServer();
                        state = State::WAITING_P2;
                    }
                }
                UI::beep();
            } else if (btn == BTN_B) {
                state = State::MENU;
                UI::beep();
            }
            break;

        case State::WAITING_P2:
            if (btn == BTN_B) {
                stopWebSocketServer();
                state = State::MODE_SELECT;
                UI::beep();
            }
            break;

        case State::PLAYING:
            if (btn == BTN_D) {
                state = State::GAME_OVER;
                playerWon = playerScore > aiScore;
            }
            break;

        case State::GAME_OVER:
            if (btn == BTN_A) {
                if (gameMode == GameMode::VS_PLAYER) {
                    // In PvP, wait for P2 ready or start if they're ready
                    if (player2Ready) {
                        player2Ready = false;
                        startGame();
                    } else {
                        // Reset for rematch, wait for P2
                        playerY = 26;
                        aiY = 26;
                        playerScore = 0;
                        aiScore = 0;
                        state = State::WAITING_P2;
                    }
                } else {
                    startGame();
                }
                UI::beep();
            } else if (btn == BTN_B || btn == BTN_D) {
                stopWebSocketServer();
                wantsToExit = true;
            }
            break;
    }
}

const uint8_t* PongApp::getIcon() {
    return icon_pong;
}
