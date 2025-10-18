#include "GameClient.h"
#include "../shared/Config.h"
#include <iostream>
#include <thread>
#include <cmath>

GameClient::GameClient(const std::string& playerName)
    : localX(Config::WORLD_WIDTH / 2.0f), localY(Config::WORLD_HEIGHT / 2.0f),
      localVx(0.0f), localVy(0.0f), fps(0), frameCount(0) {
    
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
        std::cout << "Q ability pressed (Steel Tempest)\n";
        // TODO: Send ability use to server
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
    if (inputHandler->hasTarget()) {
        std::pair<float, float> target = inputHandler->getTarget();
        float targetX = target.first;
        float targetY = target.second;
        
        // Calculate direction to target
        float dx = targetX - localX;
        float dy = targetY - localY;
        float distance = std::sqrt(dx * dx + dy * dy);
        
        // If we're close enough, stop
        const float arrivalThreshold = 3.0f;
        if (distance < arrivalThreshold) {
            inputHandler->clearTarget();
            localVx = 0.0f;
            localVy = 0.0f;
        } else {
            // Normalize direction and move
            float dirX = dx / distance;
            float dirY = dy / distance;
            
            localVx = dirX;
            localVy = dirY;
            
            // Update position (client-side prediction)
            localX += dirX * Config::PLAYER_SPEED * dt;
            localY += dirY * Config::PLAYER_SPEED * dt;
        }
    } else {
        // No target, stop moving
        localVx = 0.0f;
        localVy = 0.0f;
    }
    
    // Clamp to world bounds
    localX = (std::max)(0.0f, (std::min)(static_cast<float>(Config::WORLD_WIDTH - Config::PLAYER_SIZE), localX));
    localY = (std::max)(0.0f, (std::min)(static_cast<float>(Config::WORLD_HEIGHT - Config::PLAYER_SIZE), localY));
}

void GameClient::render() {
    renderer->clear();
    renderer->renderGrid();
    
    // Get current game state
    auto players = network->getPlayers();
    uint32_t myId = network->getPlayerId();
    
    // Render all players
    for (const auto& [playerId, player] : players) {
        bool isLocal = (playerId == myId);
        
        // Use local prediction for our player
        if (isLocal) {
            Player localPlayer = *player;
            localPlayer.x = localX;
            localPlayer.y = localY;
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
    const auto targetFrameTime = 16ms;  // ~60 FPS
    
    std::cout << "\nGame started!\n";
    std::cout << "Controls: WASD or Arrow keys to move\n";
    std::cout << "Press ESC to quit\n\n";
    
    while (!inputHandler->shouldQuit()) {
        auto currentTime = Clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastTime);
        float dt = elapsed.count() / 1000.0f;
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
