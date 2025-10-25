#include "Entities.h"
#include <algorithm>
#include <cmath>
#include <iostream>

// Player implementation (Samurai-themed)
Player::Player(uint32_t playerId, float posX, float posY, const std::string& playerName)
    : id(playerId), name(playerName), x(posX), y(posY),
      vx(0.0f), vy(0.0f), size(Config::PLAYER_SIZE), speed(Config::PLAYER_SPEED),
      isTagged(false), score(0), lastUpdate(std::chrono::steady_clock::now()),
      health(100), maxHealth(100), rotation(0.0f), isDashing(false), isAlive(true),
      activeAbility(SamuraiAbility::NONE), qStacks(0),
      hasWindWall(false), windWallRadius(60.0f),
      hasTarget(false), targetX(0.0f), targetY(0.0f) {
    
    auto now = std::chrono::steady_clock::now();
    lastQTime = now;
    lastWTime = now;
    lastETime = now;
    lastRTime = now;
    windWallStartTime = now;
}

void Player::updatePosition(float dx, float dy, float dt) {
    x += dx * speed * dt;
    y += dy * speed * dt;
    
    
}

void Player::updateVelocity(float velX, float velY) {
    vx = velX;
    vy = velY;
}

void Player::setPosition(float posX, float posY) {
    x = posX;
    y = posY;
    lastUpdate = std::chrono::steady_clock::now();
}

bool Player::checkCollision(const Player& other) const {
    float dx = x - other.x;
    float dy = y - other.y;
    float distance = std::sqrt(dx * dx + dy * dy);
    return distance < (size + other.size) / 2.0f;
}

void Player::setTarget(float tx, float ty) {
    hasTarget = true;
    targetX = tx;
    targetY = ty;
}

void Player::clearTarget() {
    hasTarget = false;
}

void Player::moveTowardsTarget(float dt) {
    if (!hasTarget) return;
    
    // Calculate direction to target
    float dx = targetX - x;
    float dy = targetY - y;
    float distance = std::sqrt(dx * dx + dy * dy);
    
    // If we're close enough to the target, stop
    const float arrivalThreshold = 3.0f;
    if (distance < arrivalThreshold) {
        clearTarget();
        vx = 0.0f;
        vy = 0.0f;
        return;
    }
    
    // Normalize direction and move
    float dirX = dx / distance;
    float dirY = dy / distance;
    
    // Update velocity
    vx = dirX;
    vy = dirY;
    
    // Update rotation to face movement direction
    rotation = std::atan2(dirY, dirX);
    
    // Update position
    updatePosition(dirX, dirY, dt);
}


bool Player::canUseQ() const {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastQTime);
    return elapsed.count() >= 400 && isAlive; 
}

bool Player::canUseW() const {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastWTime);
    return elapsed.count() >= 3000 && isAlive; 
}

bool Player::canUseE() const {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastETime);
    return elapsed.count() >= 500 && isAlive; 
}

bool Player::canUseR() const {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastRTime);
    return elapsed.count() >= 10000 && isAlive; 
}

void Player::useQ() {
    if (!canUseQ()) return;
    lastQTime = std::chrono::steady_clock::now();
    activeAbility = SamuraiAbility::Q_STEEL_TEMPEST;
    qStacks++;
    if (qStacks > 2) qStacks = 0; 
}

void Player::useW() {
    if (!canUseW()) return;
    lastWTime = std::chrono::steady_clock::now();
    activeAbility = SamuraiAbility::W_WIND_WALL;
    
    // Activate wind wall
    hasWindWall = true;
    windWallStartTime = std::chrono::steady_clock::now();
    std::cout << "Player " << id << " activated Wind Wall!" << std::endl;
}

void Player::useE(float targetX, float targetY) {
    if (!canUseE()) return;
    lastETime = std::chrono::steady_clock::now();
    activeAbility = SamuraiAbility::E_SWEEPING_BLADE;
    isDashing = true;
    
    // Dash towards target
    float dx = targetX - x;
    float dy = targetY - y;
    float distance = std::sqrt(dx * dx + dy * dy);
    if (distance > 0) {
        float dashDistance = 200.0f; // E dash range
        x += (dx / distance) * dashDistance;
        y += (dy / distance) * dashDistance;
    }
}

void Player::useR(const Player& target) {
    if (!canUseR()) return;
    lastRTime = std::chrono::steady_clock::now();
    activeAbility = SamuraiAbility::R_LAST_BREATH;
    
    // Teleport to target and deal massive damage
    x = target.x;
    y = target.y - 50; // Appear above target
}

void Player::takeDamage(int damage) {
    if (!isAlive) return;
    health -= damage;
    if (health <= 0) {
        health = 0;
        isAlive = false;
    }
}

void Player::respawn(float spawnX, float spawnY) {
    x = spawnX;
    y = spawnY;
    health = maxHealth;
    isAlive = true;
    isDashing = false;
    qStacks = 0;
    activeAbility = SamuraiAbility::NONE;
}

void Player::updateWindWall(float dt) {
    if (hasWindWall) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - windWallStartTime);
        
        // Wind wall lasts for 3.75 seconds (like in League of Legends)
        if (elapsed.count() >= 3750) {
            hasWindWall = false;
            std::cout << "Player " << id << " wind wall expired" << std::endl;
        }
    }
}

bool Player::isProjectileBlockedByWindWall(float projX, float projY) const {
    if (!hasWindWall) return false;
    
    // Calculate distance from player center to projectile
    float dx = projX - x;
    float dy = projY - y;
    float distance = std::sqrt(dx * dx + dy * dy);
    
    // Block if projectile is within wind wall radius
    return distance <= windWallRadius;
}

// Projectile implementation
Projectile::Projectile(uint32_t projId, float posX, float posY, float velX, float velY, uint32_t owner, bool tornado, int dmg)
    : id(projId), x(posX), y(posY), vx(velX), vy(velY), ownerId(owner),
      size(tornado ? 30.0f : 15.0f), speed(tornado ? 600.0f : 800.0f), active(true), 
      isTornado(tornado), damage(dmg) {
}

void Projectile::update(float dt) {
    x += vx * speed * dt;
    y += vy * speed * dt;
    
    // Deactivate if out of bounds (tornado goes farther)
    float maxDistance = isTornado ? 800.0f : 400.0f;
    if (x < -maxDistance || x > Config::WORLD_WIDTH + maxDistance || 
        y < -maxDistance || y > Config::WORLD_HEIGHT + maxDistance) {
        active = false;
    }
}

bool Projectile::checkCollision(const Player& player) const {
    if (player.id == ownerId) return false;
    
    float dx = x - player.x;
    float dy = y - player.y;
    float distance = std::sqrt(dx * dx + dy * dy);
    return distance < (size + player.size / 2.0f);
}

bool Projectile::checkCollisionWithDummy(const Dummy& dummy) const {
    if (!dummy.isAlive) return false;
    
    float dx = x - dummy.x;
    float dy = y - dummy.y;
    float distance = std::sqrt(dx * dx + dy * dy);
    return distance < (size + dummy.size / 2.0f);
}

// Dummy implementation
Dummy::Dummy(uint32_t dummyId, float posX, float posY)
    : id(dummyId), x(posX), y(posY), size(40.0f),
      health(1000), maxHealth(1000), isAlive(true) {
    lastHitTime = std::chrono::steady_clock::now();
}

void Dummy::takeDamage(int damage) {
    if (!isAlive) return;
    health -= damage;
    if (health <= 0) {
        health = 0;
        isAlive = false;
    }
    lastHitTime = std::chrono::steady_clock::now();
}

void Dummy::resetHealth() {
    health = maxHealth;
    isAlive = true;
}

bool Dummy::checkCollision(const Player& player) const {
    float dx = x - player.x;
    float dy = y - player.y;
    float distance = std::sqrt(dx * dx + dy * dy);
    return distance < (size / 2.0f + player.size / 2.0f);
}
