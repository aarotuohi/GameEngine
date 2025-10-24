#include "GameState.h"
#include <random>
#include <iostream>

GameState::GameState()
    : nextPlayerId(1), nextProjectileId(1), nextDummyId(1), taggedPlayerId(0), running(true) {
    // Spawn some initial dummies for testing
    spawnDummy(200.0f, 200.0f);
    spawnDummy(400.0f, 300.0f);
    spawnDummy(600.0f, 200.0f);
}

GameState::~GameState() {
    running = false;
}

uint32_t GameState::addPlayer(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex);
    
    uint32_t playerId = nextPlayerId++;
    
    // Random spawn position
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distX(50, Config::WORLD_WIDTH - 50);
    std::uniform_int_distribution<> distY(50, Config::WORLD_HEIGHT - 50);
    
    float x = static_cast<float>(distX(gen));
    float y = static_cast<float>(distY(gen));
    
    auto player = std::make_shared<Player>(playerId, x, y, name);
    
    // First player
    if (taggedPlayerId == 0) {
        player->isTagged = true;
        taggedPlayerId = playerId;
    }
    
    players[playerId] = player;
    std::cout << "Player " << playerId << " (" << name << ") joined at (" << x << ", " << y << ")\n";
    
    return playerId;
}

void GameState::removePlayer(uint32_t playerId) {
    std::lock_guard<std::mutex> lock(mutex);
    
    auto it = players.find(playerId);
    if (it != players.end()) {
        // Transfer tag if needed
        if (taggedPlayerId == playerId) {
            taggedPlayerId = 0;
            for (const auto& [pid, player] : players) {
                if (pid != playerId) {
                    player->isTagged = true;
                    taggedPlayerId = pid;
                    break;
                }
            }
        }
        
        players.erase(it);
        std::cout << "Player " << playerId << " left\n";
    }
}

std::shared_ptr<Player> GameState::getPlayer(uint32_t playerId) {
    std::lock_guard<std::mutex> lock(mutex);
    auto it = players.find(playerId);
    return (it != players.end()) ? it->second : nullptr;
}

void GameState::updatePlayerPosition(uint32_t playerId, float dx, float dy, float dt) {
    std::lock_guard<std::mutex> lock(mutex);
    auto it = players.find(playerId);
    if (it != players.end()) {
        it->second->updatePosition(dx, dy, dt);
    }
}

// update player velocity
void GameState::updatePlayerVelocity(uint32_t playerId, float vx, float vy) {
    std::lock_guard<std::mutex> lock(mutex);
    auto it = players.find(playerId);
    if (it != players.end()) {
        it->second->updateVelocity(vx, vy);
    }
}

//set player position
void GameState::setPlayerPosition(uint32_t playerId, float x, float y) {
    std::lock_guard<std::mutex> lock(mutex);
    auto it = players.find(playerId);
    if (it != players.end()) {
        it->second->setPosition(x, y);
    }
}

// create projectile
uint32_t GameState::createProjectile(uint32_t ownerId, float x, float y, float vx, float vy) {
    std::lock_guard<std::mutex> lock(mutex);
    uint32_t projId = nextProjectileId++;
    auto projectile = std::make_shared<Projectile>(projId, x, y, vx, vy, ownerId);
    projectiles[projId] = projectile;
    return projId;
}

uint32_t GameState::createQProjectile(uint32_t ownerId, float x, float y, float dirX, float dirY, bool isTornado, int damage) {
    std::lock_guard<std::mutex> lock(mutex);
    uint32_t projId = nextProjectileId++;
    auto projectile = std::make_shared<Projectile>(projId, x, y, dirX, dirY, ownerId, isTornado, damage);
    projectiles[projId] = projectile;
    std::cout << "Created " << (isTornado ? "TORNADO" : "blade") << " projectile for player " << ownerId << "\n";
    return projId;
}

void GameState::update(float dt) {
    std::lock_guard<std::mutex> lock(mutex);
    
    // Update players 
    for (auto& [playerId, player] : players) {
        player->updateWindWall(dt);
    }
    
    // Update projectiles
    std::vector<uint32_t> inactiveProjectiles;
    for (auto& [projId, proj] : projectiles) {
        proj->update(dt);
        if (!proj->active) {
            inactiveProjectiles.push_back(projId);
        } else {
            bool hit = false;
            
            // check is wall blocks projectile
            for (auto& [playerId, player] : players) {
                if (player->isProjectileBlockedByWindWall(proj->x, proj->y)) {
                    proj->active = false;
                    hit = true;
                    std::cout << "Projectile " << projId << " blocked by Player " << playerId << "'s Wind Wall!" << std::endl;
                    break;
                }
            }
            
            // Check collisions with dummies if not blocked
            if (!hit) {
                for (auto& [dummyId, dummy] : dummies) {
                    if (proj->checkCollisionWithDummy(*dummy)) {
                        proj->active = false;
                        hit = true;
                        dummy->takeDamage(proj->damage);
                      
                        // Stack Q
                        auto owner = players.find(proj->ownerId);
                        if (owner != players.end()) {
                            owner->second->qStacks++;
                            if (owner->second->qStacks > 2) {
                                owner->second->qStacks = 0; 
                            }
                            std::cout << "Player " << proj->ownerId << " Q stacks: " 
                                      << owner->second->qStacks << "/2\n";
                        }
                        break;
                    }
                }
            }
            
            // Check collisions with players if didn't hit dummy
            if (!hit) {
                for (auto& [playerId, player] : players) {
                    if (proj->checkCollision(*player)) {
                        proj->active = false;
                        // Award score to shooter
                        auto shooter = players.find(proj->ownerId);
                        if (shooter != players.end()) {
                            shooter->second->score++;
                        }
                        break;
                    }
                }
            }
        }
    }
    
    // Remove inactive projectiles
    for (uint32_t projId : inactiveProjectiles) {
        projectiles.erase(projId);
    }
    
    // Check player collisions 
    if (taggedPlayerId != 0) {
        auto taggedIt = players.find(taggedPlayerId);
        if (taggedIt != players.end()) {
            auto& taggedPlayer = taggedIt->second;
            for (auto& [playerId, player] : players) {
                if (playerId != taggedPlayerId) {
                    if (taggedPlayer->checkCollision(*player)) {
                        
                        taggedPlayer->isTagged = false;
                        player->isTagged = true;
                        taggedPlayer->score++;
                        taggedPlayerId = playerId;
                        std::cout << "Tag transferred to Player " << playerId << "!\n";
                        break;
                    }
                }
            }
        }
    }
}

std::vector<Protocol::PlayerState> GameState::getPlayersForBroadcast() {
    std::lock_guard<std::mutex> lock(mutex);
    std::vector<Protocol::PlayerState> states;
    
    for (const auto& [playerId, player] : players) {
        Protocol::PlayerState state;
        state.id = player->id;
        state.x = player->x;
        state.y = player->y;
        state.vx = player->vx;
        state.vy = player->vy;
        state.hasWindWall = player->hasWindWall;
        state.windWallRadius = player->windWallRadius;
        states.push_back(state);
    }
    
    return states;
}

std::unordered_map<uint32_t, std::shared_ptr<Player>> GameState::getAllPlayers() {
    std::lock_guard<std::mutex> lock(mutex);
    return players;
}

std::unordered_map<uint32_t, std::shared_ptr<Projectile>> GameState::getAllProjectiles() {
    std::lock_guard<std::mutex> lock(mutex);
    return projectiles;
}

uint32_t GameState::spawnDummy(float x, float y) {
    std::lock_guard<std::mutex> lock(mutex);
    uint32_t dummyId = nextDummyId++;
    auto dummy = std::make_shared<Dummy>(dummyId, x, y);
    dummies[dummyId] = dummy;
    return dummyId;
}

std::shared_ptr<Dummy> GameState::getDummy(uint32_t dummyId) {
    std::lock_guard<std::mutex> lock(mutex);
    auto it = dummies.find(dummyId);
    if (it != dummies.end()) {
        return it->second;
    }
    return nullptr;
}

void GameState::damageDummy(uint32_t dummyId, int damage) {
    std::lock_guard<std::mutex> lock(mutex);
    auto it = dummies.find(dummyId);
    if (it != dummies.end()) {
        it->second->takeDamage(damage);
    }
}

std::unordered_map<uint32_t, std::shared_ptr<Dummy>> GameState::getAllDummies() {
    std::lock_guard<std::mutex> lock(mutex);
    return dummies;
}
