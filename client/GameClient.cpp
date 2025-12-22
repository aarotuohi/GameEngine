#include "GameClient.h"
#include "../shared/Config.h"
#include <iostream>
#include <thread>
#include <cmath>
#include <algorithm>

GameClient::GameClient(const std::string& playerName)
    : localX(Config::WORLD_WIDTH / 2.0f), localY(Config::WORLD_HEIGHT / 2.0f),
      localVx(0.0f), localVy(0.0f), localRotation(0.0f), hasWorldTarget(false), 
    worldTargetX(0.0f), worldTargetY(0.0f), fps(60), frameCount(0), 
    isGameOver(false), shouldRestart(false), localKills(0), currentWave(1), 
    localLevel(1), localAttackDamage(20), showWaveAnnouncement(false), settingsButtonHovered(false),
    settingsMenuOpen(false), masterVolume(75.0f), musicVolume(75.0f), sfxVolume(75.0f),
    showFps(true), vsyncEnabled(true) {
    
    network = std::make_unique<NetworkManager>(playerName);
    inputHandler = std::make_unique<InputHandler>();
    renderer = std::make_unique<Renderer>();
#ifdef HAS_AUDIO_SUPPORT
    audioManager = std::make_unique<AudioManager>();
#endif
    
    positionUpdateInterval = std::chrono::duration<float>(1.0f / Config::UPDATE_RATE);
    lastPositionUpdate = std::chrono::steady_clock::now();
    lastFpsUpdate = std::chrono::steady_clock::now();
    lastQSwingTime = std::chrono::steady_clock::time_point{};
}

GameClient::~GameClient() {
}

bool GameClient::connect(const std::string& serverHost) {
    std::cout << "Connecting to " << serverHost << "...\n";
    
    if (!renderer->initialize()) {
        std::cerr << "Failed to initialize renderer\n";
        return false;
    }

#ifdef HAS_AUDIO_SUPPORT
    
    if (audioManager && audioManager->initialize()) {
       
        std::cout << "Audio system initialized\n";
    } else {
        std::cout << "Audio system not available\n";
    }
#endif
    
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
        auto now = std::chrono::steady_clock::now();
        auto timeSinceLastQ = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastQUseTime).count();
        
        if (timeSinceLastQ >= Config::Q_COOLDOWN_MS || lastQUseTime.time_since_epoch().count() == 0) {
            std::cout << "Q ability used (Steel Tempest)\n";
            
            float dirX = std::cos(localRotation);
            float dirY = std::sin(localRotation);
            network->sendAbilityUse(1, dirX, dirY); 
           
            lastQSwingTime = now;
            lastQUseTime = now;
#ifdef HAS_AUDIO_SUPPORT
            if (audioManager) {
                audioManager->playSoundEffect(SoundEffect::ABILITY_Q);
            }
#endif
            inputHandler->clearAbilityInputs();
        } else {
            std::cout << "Q on cooldown! " << (Config::Q_COOLDOWN_MS - timeSinceLastQ) / 1000.0f << "s remaining\n";
            inputHandler->clearAbilityInputs();
        }
    }
    if (inputHandler->isWPressed()) {
        auto now = std::chrono::steady_clock::now();
        auto timeSinceLastW = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastWUseTime).count();
        
        if (timeSinceLastW >= Config::W_COOLDOWN_MS || lastWUseTime.time_since_epoch().count() == 0) {
            std::cout << "W ability used (Wind Wall)\n";
           
            network->sendAbilityUse(2, 0.0f, 0.0f);
            lastWUseTime = now;
#ifdef HAS_AUDIO_SUPPORT
            if (audioManager) {
                audioManager->playSoundEffect(SoundEffect::ABILITY_W);
            }
#endif
            inputHandler->clearAbilityInputs();
        } else {
            std::cout << "W on cooldown! " << (Config::W_COOLDOWN_MS - timeSinceLastW) / 1000.0f << "s remaining\n";
            inputHandler->clearAbilityInputs();
        }
    }
    if (inputHandler->isEPressed()) {
        auto now = std::chrono::steady_clock::now();
        auto timeSinceLastE = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastEUseTime).count();
        
        if (timeSinceLastE >= Config::E_COOLDOWN_MS || lastEUseTime.time_since_epoch().count() == 0) {
            std::cout << "E ability used (Shockwave)\n";
            float dirX = std::cos(localRotation);
            float dirY = std::sin(localRotation);
            network->sendAbilityUse(3, dirX, dirY);
            lastEShockwaveTime = now;
            lastEUseTime = now;
#ifdef HAS_AUDIO_SUPPORT
            if (audioManager) {
                audioManager->playSoundEffect(SoundEffect::ABILITY_E);
            }
#endif
            inputHandler->clearAbilityInputs();
        } else {
            std::cout << "E on cooldown! " << (Config::E_COOLDOWN_MS - timeSinceLastE) / 1000.0f << "s remaining\n";
            inputHandler->clearAbilityInputs();
        }
    }
    if (inputHandler->isRPressed()) {
        auto now = std::chrono::steady_clock::now();
        auto timeSinceLastR = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastRUseTime).count();
        
        if (timeSinceLastR >= Config::R_COOLDOWN_MS || lastRUseTime.time_since_epoch().count() == 0) {
            std::cout << "R ability used (Circulating Tornadoes)\n";
            
            network->sendAbilityUse(4, 0.0f, 0.0f);
            lastRTime = now;
            lastRUseTime = now;
#ifdef HAS_AUDIO_SUPPORT
            if (audioManager) {
                audioManager->playSoundEffect(SoundEffect::ABILITY_R);
            }
#endif
            inputHandler->clearAbilityInputs();
        } else {
            std::cout << "R on cooldown! " << (Config::R_COOLDOWN_MS - timeSinceLastR) / 1000.0f << "s remaining\n";
            inputHandler->clearAbilityInputs();
        }
    }
    
    // Right-click movement system
    // input checker
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
            
            network->sendPositionUpdate(localX, localY, 0.0f, 0.0f);
            // last poistio
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


    renderer->updateCamera(localX, localY);
    
    renderer->clear();
    renderer->renderGrass();
    renderer->renderDecorations(); 
    renderer->renderGrid();
    
    auto players = network->getPlayers();
    auto enemies = network->getEnemies();
    auto projectiles = network->getProjectiles();
    auto rTornadoes = network->getRTornadoes();
    uint32_t myId = network->getPlayerId();
    
    auto myPlayerIt = players.find(myId);
    if (myPlayerIt != players.end()) {
        localKills = myPlayerIt->second->kills;
        localLevel = myPlayerIt->second->level;
        localAttackDamage = myPlayerIt->second->attackDamage;
    }
    
    currentWave = network->getCurrentWave();
    showWaveAnnouncement = network->getShowWaveAnnouncement();
    

    for (const auto& [enemyId, enemy] : enemies) {
        renderer->renderEnemy(*enemy);
    }
    
   
    for (const auto& [projId, projectile] : projectiles) {
        renderer->renderProjectile(*projectile);
    }
    

    for (const auto& [playerId, player] : players) {
        bool isLocal = (playerId == myId);
        
       
        if (isLocal) {
            Player localPlayer = *player;
            localPlayer.x = localX;
            localPlayer.y = localY;
            localPlayer.rotation = localRotation;
            renderer->renderPlayer(localPlayer, true);
            
            if (lastQSwingTime.time_since_epoch().count() > 0) {
                auto now = std::chrono::steady_clock::now();
                if (now - lastQSwingTime <= qSwingDuration) {
                    renderer->renderSwordSwing(localX, localY, localRotation,
                                               Config::Q_SWORD_ARC_DEGREES,
                                               Config::Q_SWORD_RANGE);
                }
            }

           
            if (lastEShockwaveTime.time_since_epoch().count() > 0) {
                auto now = std::chrono::steady_clock::now();
                if (now - lastEShockwaveTime <= eShockwaveVfxDuration) {
                    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastEShockwaveTime).count();
                    float progress = static_cast<float>(elapsed) / Config::E_SHOCKWAVE_VFX_MS;
                    renderer->renderShockwave(localX, localY, progress, Config::E_SHOCKWAVE_RADIUS);
                }
            }
        } else {
            renderer->renderPlayer(*player, false);
        }
    }
    
    for (const auto& [playerId, player] : players) {
        if (player->hasWindWall) {
            
            if (playerId == myId) {
                Player localPlayer = *player;
                localPlayer.x = localX;
                localPlayer.y = localY;
                renderer->renderWindWall(localPlayer);
            } else {
                renderer->renderWindWall(*player);
            }
        }
    }

    for (const auto& [tornadoId, tornado] : rTornadoes) {
       
        renderer->renderRTornado(tornado->x, tornado->y);
    }
    
    // Render UI
    renderer->renderUI(myId, static_cast<int>(players.size()), fps, localKills, currentWave, localLevel, localAttackDamage);
    
    
    renderer->renderSettingsButton(settingsButtonHovered);
    
    if (showWaveAnnouncement) {
        renderer->renderWaveAnnouncement(currentWave);
    }
    
    if (settingsMenuOpen) {
        renderer->renderSettingsMenu(settingsMenuOpen, masterVolume, musicVolume, sfxVolume, showFps, vsyncEnabled);
#ifdef HAS_AUDIO_SUPPORT
        
        if (audioManager) {
            audioManager->setMasterVolume(masterVolume);
            audioManager->setMusicVolume(musicVolume);
            audioManager->setSFXVolume(sfxVolume);
        }
#endif
    }
   
    auto now = std::chrono::steady_clock::now();
    
    float qCooldown = 0.0f;
    if (lastQUseTime.time_since_epoch().count() > 0) {
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastQUseTime).count();
        float remaining = (Config::Q_COOLDOWN_MS - elapsed) / 1000.0f;
        qCooldown = (std::max)(0.0f, remaining);
    }
    
    float wCooldown = 0.0f;
    if (lastWUseTime.time_since_epoch().count() > 0) {
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastWUseTime).count();
        float remaining = (Config::W_COOLDOWN_MS - elapsed) / 1000.0f;
        wCooldown = (std::max)(0.0f, remaining);
    }
    
    float eCooldown = 0.0f;
    if (lastEUseTime.time_since_epoch().count() > 0) {
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastEUseTime).count();
        float remaining = (Config::E_COOLDOWN_MS - elapsed) / 1000.0f;
        eCooldown = (std::max)(0.0f, remaining);
    }
    
    float rCooldown = 0.0f;
    if (lastRUseTime.time_since_epoch().count() > 0) {
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastRUseTime).count();
        float remaining = (Config::R_COOLDOWN_MS - elapsed) / 1000.0f;
        rCooldown = (std::max)(0.0f, remaining);
    }
    
    renderer->renderCooldowns(qCooldown, wCooldown, eCooldown, rCooldown);
    
 
    if (isGameOver) {
        renderer->renderGameOver();
    }
    
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
            
           
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                if (isSettingsButtonClicked()) {
                    settingsMenuOpen = !settingsMenuOpen;
                    std::cout << "Settings menu " << (settingsMenuOpen ? "opened" : "closed") << "\n";
                }
            }
            
            // Close settings menu with ESC key
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE && settingsMenuOpen) {
                settingsMenuOpen = false;
                std::cout << "Settings menu closed\n";
            }
        }
        
        // Update input
        inputHandler->update();
        
        
        updateSettingsButtonHover();
        
    
        checkGameOver();
        
    
        if (isGameOver) {
            handleGameOverInput();
            
            if (shouldRestart) {
                restartGame();
            }
        } else {
            
            updateLocalPlayer(dt);
            
          
            auto timeSinceLastUpdate = currentTime - lastPositionUpdate;
            if (timeSinceLastUpdate >= positionUpdateInterval) {
                network->sendPositionUpdate(localX, localY, localVx, localVy);
                lastPositionUpdate = currentTime;
            }
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

void GameClient::checkGameOver() {
    auto myId = network->getPlayerId();
    auto players = network->getPlayers();
    
    auto it = players.find(myId);
    if (it != players.end()) {
        auto& player = it->second;
        if (player->health <= 0 && !isGameOver) {
            isGameOver = true;
            std::cout << "GAME OVER! Your character has died.\n";
        }
    }
}

void GameClient::handleGameOverInput() {
   
    if (inputHandler->isKeyPressed(SDLK_r)) {
        shouldRestart = true;
        std::cout << "Restarting game...\n";
    }
    if (inputHandler->isKeyPressed(SDLK_ESCAPE)) {
        std::cout << "Exiting game...\n";
        inputHandler->requestQuit();
    }
    
    
    if (inputHandler->isLeftMousePressed()) {
        int mouseX = inputHandler->getMouseX();
        int mouseY = inputHandler->getMouseY();
        
        int screenWidth = 1280;  
        int screenHeight = 720;
        
        int buttonWidth = 300;
        int buttonHeight = 60;
        int button1Y = screenHeight/2 + 20;
        int button1X = screenWidth/2 - buttonWidth/2;
        
        if (isPointInRect(mouseX, mouseY, button1X, button1Y, buttonWidth, buttonHeight)) {
            shouldRestart = true;
            std::cout << "Start Game button clicked - Restarting...\n";
        }
        
        int button2Y = button1Y + 90;
        int button2X = screenWidth/2 - buttonWidth/2;
        
        if (isPointInRect(mouseX, mouseY, button2X, button2Y, buttonWidth, buttonHeight)) {
            std::cout << "Quit button clicked - Exiting...\n";
            inputHandler->requestQuit();
        }
    }
}

bool GameClient::isPointInRect(int x, int y, int rectX, int rectY, int rectW, int rectH) {
    return x >= rectX && x <= rectX + rectW && y >= rectY && y <= rectY + rectH;
}

void GameClient::restartGame() {
    isGameOver = false;
    shouldRestart = false;
    localKills = 0;
    
    // Reset local player position
    localX = Config::WORLD_WIDTH / 2.0f;
    localY = Config::WORLD_HEIGHT / 2.0f;
    localVx = 0.0f;
    localVy = 0.0f;
    localRotation = 0.0f;
    hasWorldTarget = false;
    
    // Reconnect to server (which will respawn the player)
    network->disconnect();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    network->connect(Config::SERVER_HOST);
    
    std::cout << "Game restarted!\n";
}

void GameClient::updateSettingsButtonHover() {
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    
    int buttonSize = 50;
    int margin = 10;
    int buttonX = margin;
    int buttonY = margin;
    
    settingsButtonHovered = isPointInRect(mouseX, mouseY, buttonX, buttonY, buttonSize, buttonSize);
}

bool GameClient::isSettingsButtonClicked() {
    int mouseX, mouseY;
    Uint32 mouseState = SDL_GetMouseState(&mouseX, &mouseY);
    
    int buttonSize = 50;
    int margin = 10;
    int buttonX = margin;
    int buttonY = margin;
    
    if ((mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) && 
        isPointInRect(mouseX, mouseY, buttonX, buttonY, buttonSize, buttonSize)) {
        return true;
    }
    
    return false;
}
