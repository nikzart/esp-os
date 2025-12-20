#ifndef OTA_H
#define OTA_H

#include "app.h"
#include <WebServer.h>

class OTAApp : public App {
public:
    void init() override;
    void update() override;
    void render() override;
    void onButton(uint8_t btn, bool pressed) override;
    void onClose() override;
    const char* getName() override { return "OTA Update"; }
    const uint8_t* getIcon() override;

    // Called by upload handler
    static void setProgress(int percent);
    static void setError(const char* msg);
    static void setSuccess();

private:
    enum class State {
        NO_WIFI,
        WAITING,
        UPLOADING,
        SUCCESS,
        ERROR
    };

    static State state;
    static int uploadProgress;
    static char errorMsg[48];
    
    WebServer* server;
    
    void startServer();
    void stopServer();
    
    static void handleRoot();
    static void handleUpdate();
    static void handleUpload();
};

#endif
