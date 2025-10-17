#ifndef A39AF978_4111_486D_9C4F_505B7F57E811
#define A39AF978_4111_486D_9C4F_505B7F57E811
#ifndef ENTITIES_H
#define ENTITIES_H

#include <string>
#include <cstdint>
#include <chrono>
#include "Config.h"

// Player class
class Player {
public:
    uint32_t id;
    std::string name;
    float x, y;          // Position
    float vx, vy;        // Velocity
    float size;
    float speed;
    bool isTagged;
    int score;
    std::chrono::steady_clock::time_point lastUpdate;

    Player(uint32_t playerId, float posX, float posY, const std::string& playerName = "Player");
    
    void updatePosition(float dx, float dy, float dt);
    void updateVelocity(float velX, float velY);
    void setPosition(float posX, float posY);
    bool checkCollision(const Player& other) const;
};

// Projectile class
class Projectile {
public:
    uint32_t id;
    float x, y;          // Position
    float vx, vy;        // Velocity
    uint32_t ownerId;
    float size;
    float speed;
    bool active;

    Projectile(uint32_t projId, float posX, float posY, float velX, float velY, uint32_t owner);
    
    void update(float dt);
    bool checkCollision(const Player& player) const;
};

#endif // ENTITIES_H


#endif /* A39AF978_4111_486D_9C4F_505B7F57E811 */
