#include "GameServer.h"
#include "../shared/Config.h"
#include <iostream>
#include <chrono>
#include <thread>

GameServer::GameServer() : running(false) {
    gameState = std::make_unique<GameState>();
    playerManager = std::make_unique<PlayerManager>(*gameState);
    networkHandler = std::make_unique<NetworkHandler>(*gameState, *playerManager);
}

GameServer::~GameServer() {
    stop();
}

bool GameServer::start() {
    std::cout << "Starting game server...\n";
    
    if (!networkHandler->start()) {
        std::cerr << "Failed to start network handler\n";
        return false;
    }
    
    running = true;
    gameLoopThread = std::thread(&GameServer::gameLoop, this);
    udpBroadcastThread = std::thread(&GameServer::udpBroadcastLoop, this);
    
    std::cout << "Game server running!\n";
    std::cout << "Press Ctrl+C to stop\n";
    
    return true;
}

void GameServer::gameLoop() {
    using Clock = std::chrono::steady_clock;
    using namespace std::chrono_literals;
    
    auto tickInterval = 1000ms / Config::TICK_RATE;
    auto lastTime = Clock::now();
    
    while (running) {
        auto currentTime = Clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastTime);
        float dt = elapsed.count() / 1000.0f;
        lastTime = currentTime;
        
        gameState->update(dt);
        
        auto processingTime = Clock::now() - currentTime;
        auto sleepTime = tickInterval - processingTime;
        if (sleepTime > 0ms) {
            std::this_thread::sleep_for(sleepTime);
        }
    }
}

void GameServer::udpBroadcastLoop() {
    using namespace std::chrono_literals;
    auto updateInterval = 1000ms / Config::UPDATE_RATE;
    
    while (running) {
        auto startTime = std::chrono::steady_clock::now();
        
        networkHandler->broadcastUdpState();

        auto elapsed = std::chrono::steady_clock::now() - startTime;
        auto sleepTime = updateInterval - elapsed;
        if (sleepTime > 0ms) {
            std::this_thread::sleep_for(sleepTime);
        }
    }
}

void GameServer::run() {
    using namespace std::chrono_literals;
    
    while (running) {
        std::this_thread::sleep_for(1s);
        std::cout << "Active players: " << playerManager->getPlayerCount() << "\n";
    }
}

void GameServer::stop() {
    std::cout << "\nShutting down server...\n";
    running = false;
    gameState->setRunning(false);
    
    if (networkHandler) {
        networkHandler->stop();
    }
    
    if (gameLoopThread.joinable()) gameLoopThread.join();
    if (udpBroadcastThread.joinable()) udpBroadcastThread.join();
    
    std::cout << "Server stopped\n";
}
