#ifndef A120E132_812E_4857_A5E3_8DEAFF409759
#define A120E132_812E_4857_A5E3_8DEAFF409759
#ifndef GAMECLIENT_H
#define GAMECLIENT_H

#include <memory>
#include <chrono>
#include "NetworkManager.h"
#include "InputHandler.h"
#include "Renderer.h"

class GameClient {
private:
    std::unique_ptr<NetworkManager> network;
    std::unique_ptr<InputHandler> inputHandler;
    std::unique_ptr<Renderer> renderer;
    
    // Local player state 
    float localX, localY;
    float localVx, localVy;
    float localRotation;  
    
    // Movement target
    bool hasWorldTarget;
    float worldTargetX, worldTargetY;
    
    std::chrono::steady_clock::time_point lastPositionUpdate;
    std::chrono::duration<float> positionUpdateInterval;
    
    int fps;
    std::chrono::steady_clock::time_point lastFpsUpdate;
    int frameCount;

    std::chrono::steady_clock::time_point lastQSwingTime;
    std::chrono::milliseconds qSwingDuration{180};

    std::chrono::steady_clock::time_point lastEShockwaveTime;
    std::chrono::milliseconds eShockwaveVfxDuration{Config::E_SHOCKWAVE_VFX_MS};

    std::chrono::steady_clock::time_point lastRTime;
    
    
    std::chrono::steady_clock::time_point lastQUseTime;
    std::chrono::steady_clock::time_point lastWUseTime;
    std::chrono::steady_clock::time_point lastEUseTime;
    std::chrono::steady_clock::time_point lastRUseTime;
    
    bool isGameOver;
    bool shouldRestart;
    
    int localKills;
    int currentWave;

public:
    GameClient(const std::string& playerName = "Player");
    ~GameClient();

    bool connect(const std::string& serverHost = Config::SERVER_HOST);
    void run();
    
private:
    void updateLocalPlayer(float dt);
    void render();
    void updateFps();
    void checkGameOver();
    void handleGameOverInput();
    void restartGame();
    bool isPointInRect(int x, int y, int rectX, int rectY, int rectW, int rectH);
};

#endif 


#endif
