#include "Entities.h"
#include <algorithm>
#include <cmath>

// Player implementation
Player::Player(uint32_t playerId, float posX, float posY, const std::string& playerName)
    : id(playerId), name(playerName), x(posX), y(posY),
      vx(0.0f), vy(0.0f), size(Config::PLAYER_SIZE), speed(Config::PLAYER_SPEED),
      isTagged(false), score(0), lastUpdate(std::chrono::steady_clock::now()) {
}

void Player::updatePosition(float dx, float dy, float dt) {
    x += dx * speed * dt;
    y += dy * speed * dt;
    
    // Clamp to world bounds
    x = (std::max)(0.0f, (std::min)(static_cast<float>(Config::WORLD_WIDTH) - size, x));
    y = (std::max)(0.0f, (std::min)(static_cast<float>(Config::WORLD_HEIGHT) - size, y));
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

// Projectile implementation
Projectile::Projectile(uint32_t projId, float posX, float posY, float velX, float velY, uint32_t owner)
    : id(projId), x(posX), y(posY), vx(velX), vy(velY), ownerId(owner),
      size(5.0f), speed(400.0f), active(true) {
}

void Projectile::update(float dt) {
    x += vx * speed * dt;
    y += vy * speed * dt;
    
    // Deactivate if out of bounds
    if (x < 0 || x > Config::WORLD_WIDTH || y < 0 || y > Config::WORLD_HEIGHT) {
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
