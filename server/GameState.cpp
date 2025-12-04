#include "GameState.h"
#include <random>
#include <iostream>
#include <cmath>

GameState::GameState()
    : nextPlayerId(1), nextProjectileId(1), nextEnemyId(1), nextRTornadoId(1), taggedPlayerId(0), running(true),
      currentWave(1), enemiesKilledThisWave(0), enemiesPerWave(3), waveActive(true) {

    // Spawn ONLY wave 5 regular boss for testing
    std::cout << "Spawning Wave 5 regular boss for testing at center...\n";
    spawnEnemyInternal(400.0f, 300.0f, 0, true, false);   // Regular boss (not dragon) at center
}

GameState::~GameState() {
    running = false;
}

uint32_t GameState::addPlayer(const std::string& name) {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    
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
    std::lock_guard<std::recursive_mutex> lock(mutex);
    
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
    std::lock_guard<std::recursive_mutex> lock(mutex);
    auto it = players.find(playerId);
    return (it != players.end()) ? it->second : nullptr;
}

void GameState::updatePlayerPosition(uint32_t playerId, float dx, float dy, float dt) {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    auto it = players.find(playerId);
    if (it != players.end()) {
        it->second->updatePosition(dx, dy, dt);
    }
}

// update player velocity
void GameState::updatePlayerVelocity(uint32_t playerId, float vx, float vy) {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    auto it = players.find(playerId);
    if (it != players.end()) {
        it->second->updateVelocity(vx, vy);
    }
}

//set player position
void GameState::setPlayerPosition(uint32_t playerId, float x, float y) {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    auto it = players.find(playerId);
    if (it != players.end()) {
        it->second->setPosition(x, y);
    }
}

// create projectile
uint32_t GameState::createProjectile(uint32_t ownerId, float x, float y, float vx, float vy) {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    uint32_t projId = nextProjectileId++;
    auto projectile = std::make_shared<Projectile>(projId, x, y, vx, vy, ownerId);
    projectiles[projId] = projectile;
    return projId;
}

uint32_t GameState::createQProjectile(uint32_t ownerId, float x, float y, float dirX, float dirY, bool isTornado, int damage) {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    uint32_t projId = nextProjectileId++;
    auto projectile = std::make_shared<Projectile>(projId, x, y, dirX, dirY, ownerId, isTornado, damage);
    projectiles[projId] = projectile;
    std::cout << "Created " << (isTornado ? "TORNADO" : "blade") << " projectile for player " << ownerId << "\n";
    return projId;
}

bool GameState::processQSwordSwing(uint32_t ownerId, float originX, float originY, float dirX, float dirY,
                                   float arcDegrees, float range, int damage) {
    std::lock_guard<std::recursive_mutex> lock(mutex);

    // Normalize direction
    float dirLen = std::sqrt(dirX * dirX + dirY * dirY);
    if (dirLen <= 0.0001f) {
        return false;
    }
    float ndx = dirX / dirLen;
    float ndy = dirY / dirLen;

    // Precompute angle threshold in radians
    const float halfArcRad = (arcDegrees * 0.5f) * 3.1415926535f / 180.0f;
    const float cosThreshold = std::cos(halfArcRad);
    const float rangeSq = range * range;

    bool hitEnemy = false;

    
    for (auto& [enemyId, enemy] : enemies) {
        if (!enemy->isAlive) continue;
        float vx = enemy->x - originX;
        float vy = enemy->y - originY;
        float distSq = vx * vx + vy * vy;
        if (distSq > rangeSq) continue;

        float vLen = std::sqrt(distSq);
        if (vLen <= 0.0001f) continue;
        float nvx = vx / vLen;
        float nvy = vy / vLen;

        
        float dot = ndx * nvx + ndy * nvy; 
        if (dot >= cosThreshold) {
            enemy->takeDamage(damage);
            hitEnemy = true;
            std::cout << "Player " << ownerId << " Q hit enemy " << enemyId 
                      << " for " << damage << " damage! Enemy HP: " 
                      << enemy->health << "/" << enemy->maxHealth << "\n";
        }
    }

    // Check players for hits
    for (auto& [pid, player] : players) {
        if (pid == ownerId) continue;
        if (!player->isAlive) continue;
        float vx = player->x - originX;
        float vy = player->y - originY;
        float distSq = vx * vx + vy * vy;
        if (distSq > rangeSq) continue;

        float vLen = std::sqrt(distSq);
        if (vLen <= 0.0001f) continue;
        float nvx = vx / vLen;
        float nvy = vy / vLen;

        float dot = ndx * nvx + ndy * nvy;
        if (dot >= cosThreshold) {
    
            auto ownerIt = players.find(ownerId);
            if (ownerIt != players.end()) {
                ownerIt->second->score++;
            }
          
        }
    }


    if (hitEnemy) {
        auto ownerIt = players.find(ownerId);
        if (ownerIt != players.end()) {
            ownerIt->second->qStacks++;
            if (ownerIt->second->qStacks > 2) ownerIt->second->qStacks = 0;
            std::cout << "Player " << ownerId << " Q stacks: "
                      << ownerIt->second->qStacks << "/2\n";
        }
    }

    return hitEnemy;
}

void GameState::update(float dt) {
    std::lock_guard<std::recursive_mutex> lock(mutex);

    updateRTornadoes(dt);
    updateEnemyShooting(dt);
    
    // Update players 
    for (auto& [playerId, player] : players) {
        player->updateWindWall(dt);
        player->updateRTornadoes(dt);
        
        bool isMoving = (player->vx != 0.0f || player->vy != 0.0f);
        player->updateMovementEnergy(dt, isMoving);
    }
    
    // Update projectiles
    std::vector<uint32_t> inactiveProjectiles;
    for (auto& [projId, proj] : projectiles) {
        proj->update(dt);
        if (!proj->active) {
            inactiveProjectiles.push_back(projId);
        } else {
            bool hit = false;
            
            
            for (auto& [playerId, player] : players) {
                if (player->isProjectileBlockedByWindWall(proj->x, proj->y)) {
                    proj->active = false;
                    hit = true;
                    std::cout << "Projectile " << projId << " blocked by Player " << playerId << "'s Wind Wall!" << std::endl;
                    break;
                }
            }
            
            
            if (!hit && proj->isEnemyProjectile) {
                for (auto& [playerId, player] : players) {
                    if (!player->isAlive) continue;
                    if (proj->checkCollision(*player)) {
                        proj->active = false;
                        hit = true;
                        player->takeDamage(proj->damage);
                        std::cout << "Enemy bullet hit Player " << playerId << " for " << proj->damage << " damage! HP: " << player->health << "/" << player->maxHealth << "\n";
                        break;
                    }
                }
            }
        
            if (!hit && !proj->isEnemyProjectile) {
                for (auto& [enemyId, enemy] : enemies) {
                    if (!enemy->isAlive) continue; 
                    if (proj->checkCollisionWithEnemy(*enemy)) {
                        proj->active = false;
                        hit = true;
                        enemy->takeDamage(proj->damage, proj->ownerId);
                
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
            
           
            if (!hit && !proj->isEnemyProjectile) {
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
    
    for (uint32_t projId : inactiveProjectiles) {
        projectiles.erase(projId);
    }
    
    std::vector<uint32_t> deadEnemies;
    for (auto& [enemyId, enemy] : enemies) {
        if (!enemy->isAlive) {
            deadEnemies.push_back(enemyId);
            
            if (enemy->lastDamagedBy != 0) {
                auto killer = players.find(enemy->lastDamagedBy);
                if (killer != players.end()) {
                    killer->second->kills += enemy->killValue;
                    enemiesKilledThisWave++;
                    std::cout << "Player " << enemy->lastDamagedBy << " killed " 
                              << (enemy->isBoss ? "BOSS" : "Enemy") << " " << enemyId 
                              << " (+" << enemy->killValue << " kills)! Total kills: " 
                              << killer->second->kills << "\n";
                }
            }
        }
    }
    
    for (uint32_t enemyId : deadEnemies) {
        enemies.erase(enemyId);
    }
    
    if (waveActive && enemies.empty()) {
        waveActive = false;
        currentWave++;
        enemiesKilledThisWave = 0;
        
        
        for (auto& [playerId, player] : players) {
            player->level++;
            player->maxHealth += 50;  
            player->health = player->maxHealth;  
            player->attackDamage += 5;  
            std::cout << "Player " << player->name << " leveled up to " << player->level 
                      << " (HP: " << player->maxHealth << ", AD: " << player->attackDamage << ")\n";
        }
        
        
      
        bool isBossWave = (currentWave % 5 == 0);
        bool isDragonWave = (currentWave == 10);
        
        if (isDragonWave) {
            std::cout << "*** DRAGON RAID BOSS! ***\n";
            spawnEnemyInternal(Config::WORLD_WIDTH / 2.0f, Config::WORLD_HEIGHT / 2.0f, 0, true, true);
        } else if (isBossWave) {
            std::cout << "*** BOSS WAVE! ***\n";
            spawnEnemyInternal(Config::WORLD_WIDTH / 2.0f, Config::WORLD_HEIGHT / 2.0f, 0, true);
        } else {
            
            enemiesPerWave = 3 + (currentWave - 1);
            if (enemiesPerWave > 15) enemiesPerWave = 15;
    
            
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> distX(50, Config::WORLD_WIDTH - 50);
            std::uniform_int_distribution<> distY(50, Config::WORLD_HEIGHT - 50);
            
            for (int i = 0; i < enemiesPerWave; i++) {
                float spawnX = static_cast<float>(distX(gen));
                float spawnY = static_cast<float>(distY(gen));
                spawnEnemyInternal(spawnX, spawnY);
            }
        }
        
        waveActive = true;
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

void GameState::processEShockwave(uint32_t ownerId, float centerX, float centerY, float radius, int damage) {
  
    float r2 = radius * radius;

    for (auto& [enemyId, enemy] : enemies) {
        if (!enemy->isAlive) continue;
        float dx = enemy->x - centerX;
        float dy = enemy->y - centerY;
        if (dx*dx + dy*dy <= r2) {
            enemy->takeDamage(damage);
            std::cout << "E shockwave hit enemy " << enemyId << " for " << damage << " damage\n";
        }
    }

   
    for (auto& [pid, player] : players) {
        if (pid == ownerId) continue;
        if (!player->isAlive) continue;
        float dx = player->x - centerX;
        float dy = player->y - centerY;
        if (dx*dx + dy*dy <= r2) {
            auto ownerIt = players.find(ownerId);
            if (ownerIt != players.end()) {
                ownerIt->second->score++;
            }
        }
    }
}

std::vector<Protocol::PlayerState> GameState::getPlayersForBroadcast() {
    std::lock_guard<std::recursive_mutex> lock(mutex);
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
        state.health = player->health;
        state.maxHealth = player->maxHealth;
        state.kills = player->kills;
        state.level = player->level;
        state.attackDamage = player->attackDamage;
        state.movementEnergy = player->movementEnergy;
        state.shieldHealth = player->shieldHealth;
        state.maxShieldHealth = player->maxShieldHealth;
        states.push_back(state);
    }
    
    return states;
}

std::unordered_map<uint32_t, std::shared_ptr<Player>> GameState::getAllPlayers() {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    return players;
}

std::unordered_map<uint32_t, std::shared_ptr<Projectile>> GameState::getAllProjectiles() {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    return projectiles;
}

uint32_t GameState::spawnEnemy(float x, float y, uint32_t targetPlayerId) {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    return spawnEnemyInternal(x, y, targetPlayerId, false);
}

uint32_t GameState::spawnEnemyInternal(float x, float y, uint32_t targetPlayerId, bool isBoss, bool isDragon) {
    
    uint32_t enemyId = nextEnemyId++;
    auto enemy = std::make_shared<Enemy>(enemyId, x, y, targetPlayerId, isBoss || isDragon);
    
 
    if (isDragon) {
        enemy->isDragon = true;
        enemy->size = 120.0f;  
        enemy->health = 2000;  
        enemy->maxHealth = 2000;
        enemy->killValue = 25;  
    }
    
    float hpMultiplier = 1.0f + (currentWave - 1) * 0.1f;
    enemy->health = static_cast<int>(enemy->health * hpMultiplier);
    enemy->maxHealth = static_cast<int>(enemy->maxHealth * hpMultiplier);
    
    enemies[enemyId] = enemy;
    if (isDragon) {
        std::cout << "Spawned DRAGON RAID BOSS " << enemyId << " at (" << x << ", " << y << ") with " << enemy->maxHealth << " HP!\n";
    } else if (isBoss) {
        std::cout << "Spawned BOSS enemy " << enemyId << " at (" << x << ", " << y << ") with " << enemy->maxHealth << " HP!\n";
    } else {
        std::cout << "Spawned enemy " << enemyId << " at (" << x << ", " << y << ") with " << enemy->maxHealth << " HP\n";
    }
    return enemyId;
}

std::shared_ptr<Enemy> GameState::getEnemy(uint32_t enemyId) {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    auto it = enemies.find(enemyId);
    if (it != enemies.end()) {
        return it->second;
    }
    return nullptr;
}

void GameState::damageEnemy(uint32_t enemyId, int damage) {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    auto it = enemies.find(enemyId);
    if (it != enemies.end()) {
        it->second->takeDamage(damage);
    }
}

std::unordered_map<uint32_t, std::shared_ptr<Enemy>> GameState::getAllEnemies() {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    
    std::unordered_map<uint32_t, std::shared_ptr<Enemy>> aliveEnemies;
    for (const auto& [enemyId, enemy] : enemies) {
        if (enemy->isAlive) {
            aliveEnemies[enemyId] = enemy;
        }
    }
    return aliveEnemies;
}

void GameState::updateEnemyShooting(float dt) {

    
    const float separationDistance = 60.0f; 
    const float separationStrength = 100.0f; 
    
   
    for (auto& [enemyId, enemy] : enemies) {
        if (!enemy->isAlive) continue;
        
       
        if (std::isnan(enemy->x) || std::isnan(enemy->y)) {
            std::cout << "WARNING: Enemy " << enemyId << " has NaN position! Resetting...\n";
            enemy->x = Config::WORLD_WIDTH / 2.0f;
            enemy->y = Config::WORLD_HEIGHT / 2.0f;
            enemy->vx = 0.0f;
            enemy->vy = 0.0f;
            continue;
        }
        
        float separationX = 0.0f;
        float separationY = 0.0f;
        
        for (auto& [otherId, other] : enemies) {
            if (otherId == enemyId || !other->isAlive) continue;
            
            float dx = enemy->x - other->x;
            float dy = enemy->y - other->y;
            float dist = std::sqrt(dx * dx + dy * dy);
            
            if (dist < separationDistance && dist > 0.001f) {
                float force = (separationDistance - dist) / separationDistance;
                separationX += (dx / dist) * force;
                separationY += (dy / dist) * force;
            }
        }

        if (separationX != 0.0f || separationY != 0.0f) {
            enemy->x += separationX * separationStrength * dt;
            enemy->y += separationY * separationStrength * dt;
            
            // Clamp to world bounds
            enemy->x = (std::max)(10.0f, (std::min)(enemy->x, static_cast<float>(Config::WORLD_WIDTH) - 10.0f));
            enemy->y = (std::max)(10.0f, (std::min)(enemy->y, static_cast<float>(Config::WORLD_HEIGHT) - 10.0f));
        }
    }
    
    
    for (auto& [enemyId, enemy] : enemies) {
        if (!enemy->isAlive) continue;

        std::shared_ptr<Player> nearestPlayer = nullptr;
        float nearestDist = 999999.0f;
        
        for (auto& [playerId, player] : players) {
            if (!player->isAlive) continue;
            
            float dx = player->x - enemy->x;
            float dy = player->y - enemy->y;
            float dist = std::sqrt(dx * dx + dy * dy);
            
            if (dist < nearestDist) {
                nearestDist = dist;
                nearestPlayer = player;
            }
        }
        
        if (nearestPlayer) {
            enemy->moveTowards(nearestPlayer->x, nearestPlayer->y, dt);
        }
        
        if (nearestPlayer && enemy->canShoot() && nearestDist <= Config::ENEMY_SHOOT_RANGE) {
            float dx = nearestPlayer->x - enemy->x;
            float dy = nearestPlayer->y - enemy->y;
            float dist = std::sqrt(dx * dx + dy * dy);
            
            if (dist > 0.0f) {
            
                float dirX = dx / dist;
                float dirY = dy / dist;
                
          
                enemy->rotation = std::atan2(dirY, dirX);
            
              
                float damageMultiplier = 1.0f + (currentWave - 1) * 0.05f;
                int scaledDamage = static_cast<int>(Config::ENEMY_BULLET_DAMAGE * damageMultiplier);
                
           
                if (enemy->isDragon) {
                    scaledDamage = static_cast<int>(scaledDamage * 2.0f); 
                }
                
                uint32_t projId = nextProjectileId++;
                auto projectile = std::make_shared<Projectile>(
                    projId, enemy->x, enemy->y, dirX, dirY, 
                    enemyId, false, scaledDamage, true  
                );
                
              
                if (enemy->isDragon) {
                    projectile->isFireball = true;
                    projectile->size = 25.0f;  
                    projectile->speed = 500.0f;  
                }
                
                projectiles[projId] = projectile;
                
                enemy->shoot();
                if (enemy->isDragon) {
                    std::cout << "Dragon " << enemyId << " shot FIREBALL at player " << nearestPlayer->id << " for " << scaledDamage << " damage!\n";
                } else {
                    std::cout << "Enemy " << enemyId << " shot laser at player " << nearestPlayer->id << "\n";
                }
            }
        }
    }
}

void GameState::createRTornadoes(uint32_t ownerId) {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    
    auto playerIt = players.find(ownerId);
    if (playerIt == players.end()) {
        std::cout << "ERROR: Cannot create R tornadoes - player " << ownerId << " not found!\n";
        return;
    }
    
    float angleStep = (2.0f * 3.14159265f) / static_cast<float>(Config::R_TORNADO_COUNT);
    
    for (int i = 0; i < Config::R_TORNADO_COUNT; i++) {
        uint32_t tornadoId = nextRTornadoId++;
        float angle = angleStep * static_cast<float>(i);
        auto tornado = std::make_shared<RTornado>(tornadoId, ownerId, angle, i);
        rTornadoes[tornadoId] = tornado;
        
        
    }
    
    std::cout << "Created " << Config::R_TORNADO_COUNT << " R tornadoes for player " << ownerId << "\n";
}

void GameState::updateRTornadoes(float dt) {
    
    std::vector<uint32_t> tornadoesToRemove;
    
    for (auto& [tornadoId, tornado] : rTornadoes) {
        auto ownerIt = players.find(tornado->ownerId);
        if (ownerIt == players.end() || !ownerIt->second->hasRTornadoes) {
            tornadoesToRemove.push_back(tornadoId);
            continue;
        }
        
        auto& owner = ownerIt->second;
        
        if (std::isnan(owner->rTornadoAngle)) {
            std::cout << "WARNING: Player " << tornado->ownerId << " has NaN rTornadoAngle! Resetting...\n";
            owner->rTornadoAngle = 0.0f;
        }
        
        tornado->angle = owner->rTornadoAngle + (2.0f * 3.14159265f / Config::R_TORNADO_COUNT) * tornado->tornadoIndex;
        
    
        if (std::isnan(tornado->angle)) {
            std::cout << "WARNING: Tornado " << tornado->id << " has NaN angle! Resetting...\n";
            tornado->angle = 0.0f;
        }
        
        float tornadoX = owner->x + std::cos(tornado->angle) * Config::R_ORBIT_RADIUS;
        float tornadoY = owner->y + std::sin(tornado->angle) * Config::R_ORBIT_RADIUS;
        
      
        auto now = std::chrono::steady_clock::now();
        auto timeSinceLastHit = std::chrono::duration_cast<std::chrono::milliseconds>(now - tornado->lastHitTime);
        
        if (timeSinceLastHit.count() >= 500) { 
            for (auto& [enemyId, enemy] : enemies) {
                if (!enemy->isAlive) continue;
                
                float dx = enemy->x - tornadoX;
                float dy = enemy->y - tornadoY;
                float distSq = dx * dx + dy * dy;
                float hitRadius = static_cast<float>(Config::R_TORNADO_SIZE);
                
                if (distSq <= hitRadius * hitRadius) {
                   
                    int tornadoDamage = Config::R_TORNADO_DAMAGE;
                    auto ownerIt = players.find(tornado->ownerId);
                    if (ownerIt != players.end()) {
                        tornadoDamage = static_cast<int>(ownerIt->second->attackDamage * 1.5f);
                    }
                    enemy->takeDamage(tornadoDamage);
                    tornado->lastHitTime = now;
                    std::cout << "R tornado hit enemy " << enemyId << " for " << tornadoDamage << " damage\n";
                    break; 
                }
            }
            
            // Check collisions with other players
            for (auto& [pid, player] : players) {
                if (pid == tornado->ownerId || !player->isAlive) continue;
                
                float dx = player->x - tornadoX;
                float dy = player->y - tornadoY;
                float distSq = dx * dx + dy * dy;
                float hitRadius = static_cast<float>(Config::R_TORNADO_SIZE);
                
                if (distSq <= hitRadius * hitRadius) {
                  
                    owner->score++;
                    tornado->lastHitTime = now;
                    std::cout << "R tornado hit player " << pid << "\n";
                    break; 
                }
            }
        }
    }
    
    // Remove expired tornadoes
    for (uint32_t tid : tornadoesToRemove) {
        rTornadoes.erase(tid);
    }
}

void GameState::removeRTornadoes(uint32_t ownerId) {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    
    std::vector<uint32_t> toRemove;
    for (auto& [tornadoId, tornado] : rTornadoes) {
        if (tornado->ownerId == ownerId) {
            toRemove.push_back(tornadoId);
        }
    }
    
    for (uint32_t tid : toRemove) {
        rTornadoes.erase(tid);
    }
}

std::unordered_map<uint32_t, std::shared_ptr<RTornado>> GameState::getAllRTornadoes() {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    return rTornadoes;
}
