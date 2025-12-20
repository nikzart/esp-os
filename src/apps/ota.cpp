#include "apps/ota.h"
#include "ui.h"
#include "config.h"
#include "wifi_manager.h"
#include "icons.h"
#include <Update.h>

extern U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2;

// Static member definitions
OTAApp::State OTAApp::state = OTAApp::State::WAITING;
int OTAApp::uploadProgress = 0;
char OTAApp::errorMsg[48] = {0};

// HTML page for upload
static const char* uploadPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>ESP32 OTA Update</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: Arial; text-align: center; padding: 20px; background: #1a1a2e; color: #eee; }
        h1 { color: #00d4ff; }
        .upload-form { background: #16213e; padding: 30px; border-radius: 10px; max-width: 400px; margin: 20px auto; }
        input[type="file"] { margin: 20px 0; }
        input[type="submit"] { background: #00d4ff; color: #000; padding: 12px 30px; border: none; border-radius: 5px; cursor: pointer; font-size: 16px; }
        input[type="submit"]:hover { background: #00a8cc; }
        .progress { width: 100%; background: #333; border-radius: 5px; margin: 20px 0; display: none; }
        .progress-bar { width: 0%; height: 30px; background: #00d4ff; border-radius: 5px; transition: width 0.3s; }
        .status { margin: 20px 0; font-size: 14px; }
    </style>
</head>
<body>
    <h1>ESP32 OTA Update</h1>
    <div class="upload-form">
        <form method="POST" action="/update" enctype="multipart/form-data" id="upload-form">
            <p>Select firmware file (.bin)</p>
            <input type="file" name="firmware" accept=".bin" required>
            <br>
            <input type="submit" value="Upload & Update">
        </form>
        <div class="progress" id="progress">
            <div class="progress-bar" id="progress-bar"></div>
        </div>
        <div class="status" id="status"></div>
    </div>
    <script>
        document.getElementById('upload-form').addEventListener('submit', function(e) {
            e.preventDefault();
            var form = e.target;
            var formData = new FormData(form);
            var xhr = new XMLHttpRequest();
            
            document.getElementById('progress').style.display = 'block';
            document.getElementById('status').innerText = 'Uploading...';
            
            xhr.upload.addEventListener('progress', function(e) {
                if (e.lengthComputable) {
                    var percent = Math.round((e.loaded / e.total) * 100);
                    document.getElementById('progress-bar').style.width = percent + '%';
                    document.getElementById('status').innerText = 'Uploading: ' + percent + '%';
                }
            });
            
            xhr.onload = function() {
                if (xhr.status === 200) {
                    document.getElementById('status').innerText = 'Update successful! Rebooting...';
                    document.getElementById('progress-bar').style.background = '#00ff00';
                } else {
                    document.getElementById('status').innerText = 'Error: ' + xhr.responseText;
                    document.getElementById('progress-bar').style.background = '#ff0000';
                }
            };
            
            xhr.onerror = function() {
                document.getElementById('status').innerText = 'Upload failed!';
                document.getElementById('progress-bar').style.background = '#ff0000';
            };
            
            xhr.open('POST', '/update');
            xhr.send(formData);
        });
    </script>
</body>
</html>
)rawliteral";

// Static WebServer pointer for handlers
static WebServer* serverPtr = nullptr;

void OTAApp::init() {
    state = State::WAITING;
    uploadProgress = 0;
    errorMsg[0] = '\0';
    server = nullptr;
    
    if (!WiFiManager::isConnected()) {
        state = State::NO_WIFI;
        return;
    }
    
    startServer();
}

void OTAApp::startServer() {
    server = new WebServer(80);
    serverPtr = server;
    
    server->on("/", HTTP_GET, handleRoot);
    server->on("/update", HTTP_POST, handleUpdate, handleUpload);
    
    server->begin();
    Serial.println("OTA server started");
}

void OTAApp::stopServer() {
    if (server) {
        server->stop();
        delete server;
        server = nullptr;
        serverPtr = nullptr;
        Serial.println("OTA server stopped");
    }
}

void OTAApp::handleRoot() {
    if (serverPtr) {
        serverPtr->send(200, "text/html", uploadPage);
    }
}

void OTAApp::handleUpdate() {
    if (!serverPtr) return;
    
    if (Update.hasError()) {
        serverPtr->send(500, "text/plain", "Update failed!");
    } else {
        serverPtr->send(200, "text/plain", "Update successful! Rebooting...");
        delay(1000);
        ESP.restart();
    }
}

void OTAApp::handleUpload() {
    if (!serverPtr) return;
    
    HTTPUpload& upload = serverPtr->upload();
    
    if (upload.status == UPLOAD_FILE_START) {
        Serial.printf("Update: %s\n", upload.filename.c_str());
        state = State::UPLOADING;
        uploadProgress = 0;
        
        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
            Update.printError(Serial);
            setError("Update begin failed");
        }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
            Update.printError(Serial);
            setError("Write failed");
        } else {
            // Calculate progress
            if (upload.totalSize > 0) {
                uploadProgress = (upload.currentSize * 100) / upload.totalSize;
            }
            uploadProgress = min(uploadProgress + 1, 99);
        }
    } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) {
            Serial.printf("Update Success: %u bytes\n", upload.totalSize);
            setSuccess();
        } else {
            Update.printError(Serial);
            setError("Update end failed");
        }
    }
}

void OTAApp::setProgress(int percent) {
    uploadProgress = percent;
}

void OTAApp::setError(const char* msg) {
    state = State::ERROR;
    strncpy(errorMsg, msg, sizeof(errorMsg) - 1);
}

void OTAApp::setSuccess() {
    state = State::SUCCESS;
    uploadProgress = 100;
}

void OTAApp::update() {
    if (server) {
        server->handleClient();
    }
}

void OTAApp::render() {
    UI::clear();
    UI::drawTitleBar("OTA Update");
    
    u8g2.setFont(u8g2_font_5x7_tf);
    
    switch (state) {
        case State::NO_WIFI:
            u8g2.drawStr(2, 25, "WiFi not connected!");
            u8g2.drawStr(2, 36, "Connect to WiFi first");
            u8g2.drawStr(2, 47, "in Settings app.");
            break;
            
        case State::WAITING: {
            u8g2.drawStr(2, 20, "Server running!");
            u8g2.drawStr(2, 30, "Open in browser:");

            IPAddress ip = WiFiManager::getIP();
            char ipStr[32];
            snprintf(ipStr, sizeof(ipStr), "http://%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
            u8g2.drawStr(2, 42, ipStr);

            u8g2.drawStr(2, 54, "Then upload .bin file");
            break;
        }
        
        case State::UPLOADING: {
            u8g2.drawStr(2, 25, "Uploading firmware...");
            
            // Progress bar
            u8g2.drawFrame(2, 35, 124, 12);
            int barWidth = (uploadProgress * 120) / 100;
            u8g2.drawBox(4, 37, barWidth, 8);
            
            char progStr[16];
            snprintf(progStr, sizeof(progStr), "%d%%", uploadProgress);
            u8g2.drawStr(58, 57, progStr);
            break;
        }
        
        case State::SUCCESS:
            u8g2.drawStr(2, 30, "Update successful!");
            u8g2.drawStr(2, 42, "Rebooting...");
            break;
            
        case State::ERROR:
            u8g2.drawStr(2, 25, "Update failed!");
            u8g2.drawStr(2, 38, errorMsg);
            u8g2.drawStr(2, 51, "Press B to go back");
            break;
    }
    
    UI::setNormalFont();
    
    if (state != State::UPLOADING && state != State::SUCCESS) {
        UI::drawStatusBar("", "B:Back");
    }
    
    UI::flush();
}

void OTAApp::onButton(uint8_t btn, bool pressed) {
    if (!pressed) return;
    
    if (btn == BTN_B && state != State::UPLOADING && state != State::SUCCESS) {
        wantsToExit = true;
    }
}

void OTAApp::onClose() {
    stopServer();
}

const uint8_t* OTAApp::getIcon() {
    return icon_ota;
}
