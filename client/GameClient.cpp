#include "GameClient.h"
#include "../shared/Config.h"
#include <iostream>
#include <thread>
#include <cmath>

GameClient::GameClient(const std::string& playerName)
    : localX(Config::WORLD_WIDTH / 2.0f), localY(Config::WORLD_HEIGHT / 2.0f),
      localVx(0.0f), localVy(0.0f), localRotation(0.0f), hasWorldTarget(false), 
      worldTargetX(0.0f), worldTargetY(0.0f), fps(60), frameCount(0) {
    
    network = std::make_unique<NetworkManager>(playerName);
    inputHandler = std::make_unique<InputHandler>();
    renderer = std::make_unique<Renderer>();
    
    positionUpdateInterval = std::chrono::duration<float>(1.0f / Config::UPDATE_RATE);
    lastPositionUpdate = std::chrono::steady_clock::now();
    lastFpsUpdate = std::chrono::steady_clock::now();
}

GameClient::~GameClient() {
}

bool GameClient::connect(const std::string& serverHost) {
    std::cout << "Connecting to " << serverHost << "...\n";
    
    if (!renderer->initialize()) {
        std::cerr << "Failed to initialize renderer\n";
        return false;
    }
    
    if (!network->connect(serverHost)) {
        std::cerr << "Failed to connect to server\n";
        return false;
    }
    
    std::cout << "Connected successfully!\n";
    return true;
}

void GameClient::updateLocalPlayer(float dt) {
    // Check for ability key presses
    if (inputHandler->isQPressed()) {
        std::cout << "Q ability used (Steel Tempest)\n";
        // Send Q ability to server (direction based on character rotation)
        float dirX = std::cos(localRotation);
        float dirY = std::sin(localRotation);
        network->sendAbilityUse(1, dirX, dirY);  // 1 = Q ability
        inputHandler->clearAbilityInputs();
    }
    if (inputHandler->isWPressed()) {
        std::cout << "W ability pressed (Wind Wall)\n";
        // TODO: Send ability use to server
        inputHandler->clearAbilityInputs();
    }
    if (inputHandler->isEPressed()) {
        std::cout << "E ability pressed (Sweeping Blade)\n";
        // TODO: Send ability use to server
        inputHandler->clearAbilityInputs();
    }
    if (inputHandler->isRPressed()) {
        std::cout << "R ability pressed (Last Breath)\n";
        // TODO: Send ability use to server
        inputHandler->clearAbilityInputs();
    }
    
    // Right-click movement system
    // Check if user clicked a new target
    if (inputHandler->hasTarget() && !hasWorldTarget) {
        // New target clicked
        std::pair<float, float> screenTarget = inputHandler->getTarget();
        renderer->screenToWorld(static_cast<int>(screenTarget.first), 
                               static_cast<int>(screenTarget.second), 
                               worldTargetX, worldTargetY);
        hasWorldTarget = true;
        inputHandler->clearTarget();
    }
    // Copied from github
    if (hasWorldTarget) {
        // Calculate direction to target using stored world coordinates
        float dx = worldTargetX - localX;
        float dy = worldTargetY - localY;
        float distance = std::sqrt(dx * dx + dy * dy);
        
        // If we're close enough, stop
        const float arrivalThreshold = 5.0f;
        if (distance < arrivalThreshold) {
            // Snap to target position and stop
            localX = worldTargetX;
            localY = worldTargetY;
            localVx = 0.0f;
            localVy = 0.0f;
            hasWorldTarget = false;
            // Immediately send stop command to server
            network->sendPositionUpdate(localX, localY, 0.0f, 0.0f);
            // Update timestamp to prevent immediate regular update from overriding
            lastPositionUpdate = std::chrono::steady_clock::now();
        } else {
            // Normalize direction and move
            float dirX = dx / distance;
            float dirY = dy / distance;
            
            localVx = dirX;
            localVy = dirY;
            
            // Calculate target rotation 
            float targetRotation = std::atan2(dirY, dirX);
            
            // Smoothly interpolate rotation towards target
            const float rotationSpeed = 10.0f; 
            float rotationDiff = targetRotation - localRotation;
            
            // Normalize angle 
            while (rotationDiff > 3.14159f) rotationDiff -= 2.0f * 3.14159f;
            while (rotationDiff < -3.14159f) rotationDiff += 2.0f * 3.14159f;
            
            // Apply smooth rotation
            float rotationChange = rotationSpeed * dt;
            if (std::abs(rotationDiff) < rotationChange) {
                localRotation = targetRotation;
            } else {
                localRotation += (rotationDiff > 0 ? rotationChange : -rotationChange);
            }
            
            // Update position (client-side prediction)
            localX += dirX * Config::PLAYER_SPEED * dt;
            localY += dirY * Config::PLAYER_SPEED * dt;
        }
    } else {
        // No target, stop moving
        localVx = 0.0f;
        localVy = 0.0f;
    }
    
    
}

void GameClient::render() {

    // Update camera to follow local player
    renderer->updateCamera(localX, localY);
    
    renderer->clear();
    renderer->renderGrass();
    renderer->renderDecorations(); // Cabins, spruces, and campfires
    renderer->renderGrid();
    
    // Get current game state
    auto players = network->getPlayers();
    auto dummies = network->getDummies();
    uint32_t myId = network->getPlayerId();
    
    // Render all dummies
    for (const auto& [dummyId, dummy] : dummies) {
        renderer->renderDummy(*dummy);
    }
    
    // Render all players
    for (const auto& [playerId, player] : players) {
        bool isLocal = (playerId == myId);
        
        // Use local prediction for our player
        if (isLocal) {
            Player localPlayer = *player;
            localPlayer.x = localX;
            localPlayer.y = localY;
            localPlayer.rotation = localRotation;
            renderer->renderPlayer(localPlayer, true);
        } else {
            renderer->renderPlayer(*player, false);
        }
    }
    
    // Render UI
    renderer->renderUI(myId, static_cast<int>(players.size()), fps);
    
    renderer->present();
}

void GameClient::updateFps() {
    frameCount++;
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - lastFpsUpdate);
    
    if (elapsed.count() >= 1) {
        fps = frameCount;
        frameCount = 0;
        lastFpsUpdate = now;
    }
}

void GameClient::run() {
    using Clock = std::chrono::steady_clock;
    using namespace std::chrono_literals;
    
    auto lastTime = Clock::now();
    const auto targetFrameTime = 7ms;  
    
    std::cout << "\nGame started!\n";
    std::cout << "Controls: Right click to move character\n";
    std::cout << "Press ESC to quit\n\n";
    
    while (!inputHandler->shouldQuit()) {
        auto currentTime = Clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastTime);
        float dt = elapsed.count() / 1000.0f;
        
        // Cap delta time to prevent large jumps 
        if (dt > 0.033f) dt = 0.033f;
        
        lastTime = currentTime;
        
        // Handle events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            inputHandler->handleEvent(event);
        }
        
        // Update input
        inputHandler->update();
        
        // Update local player
        updateLocalPlayer(dt);
        
        // Send position updates
        auto timeSinceLastUpdate = currentTime - lastPositionUpdate;
        if (timeSinceLastUpdate >= positionUpdateInterval) {
            network->sendPositionUpdate(localX, localY, localVx, localVy);
            lastPositionUpdate = currentTime;
        }
        
        // Render
        render();
        updateFps();
        
        // Frame rate limiting
        auto frameTime = Clock::now() - currentTime;
        if (frameTime < targetFrameTime) {
            std::this_thread::sleep_for(targetFrameTime - frameTime);
        }
    }
    
    std::cout << "Shutting down client...\n";
}
