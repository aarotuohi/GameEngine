#ifndef A39AF978_4111_486D_9C4F_505B7F57E811
#define A39AF978_4111_486D_9C4F_505B7F57E811
#ifndef ENTITIES_H
#define ENTITIES_H

#include <string>
#include <cstdint>
#include <chrono>
#include "Config.h"

// Samurai ability states
enum class SamuraiAbility {
    NONE = 0,
    Q_STEEL_TEMPEST = 1,    // Linear dash/slash
    W_WIND_WALL = 2,         // Projectile blocking
    E_SWEEPING_BLADE = 3,    // Dash through enemies
    R_LAST_BREATH = 4        // Ultimate
};

// Player class (Samurai-themed)
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

    // Samurai-specific attributes
    int health;
    int maxHealth;
    float rotation;      // Character facing direction
    bool isDashing;
    bool isAlive;
    SamuraiAbility activeAbility;
    std::chrono::steady_clock::time_point lastQTime;
    std::chrono::steady_clock::time_point lastWTime;
    std::chrono::steady_clock::time_point lastETime;
    std::chrono::steady_clock::time_point lastRTime;
    int qStacks;         // Steel Tempest stacks (0-2, third cast is tornado)
    
    // Movement target (right-click movement)
    bool hasTarget;
    float targetX, targetY;
    
    Player(uint32_t playerId, float posX, float posY, const std::string& playerName = "Player");
    
    void updatePosition(float dx, float dy, float dt);
    void updateVelocity(float velX, float velY);
    void setPosition(float posX, float posY);
    bool checkCollision(const Player& other) const;
    
    // Movement target methods
    void setTarget(float tx, float ty);
    void clearTarget();
    void moveTowardsTarget(float dt);

    // Samurai abilities
    bool canUseQ() const;
    bool canUseW() const;
    bool canUseE() const;
    bool canUseR() const;
    void useQ();
    void useW();
    void useE(float targetX, float targetY);
    void useR(const Player& target);
    void takeDamage(int damage);
    void respawn(float spawnX, float spawnY);
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
    bool isTornado;      // True if this is a Q3 tornado
    int damage;          // Damage dealt

    Projectile(uint32_t projId, float posX, float posY, float velX, float velY, uint32_t owner, bool tornado = false, int dmg = 20);
    
    void update(float dt);
    bool checkCollision(const Player& player) const;
    bool checkCollisionWithDummy(const Dummy& dummy) const;
};

// Target Dummy class for ability testing
class Dummy {
public:
    uint32_t id;
    float x, y;          // Position
    float size;
    int health;
    int maxHealth;
    bool isAlive;
    std::chrono::steady_clock::time_point lastHitTime;
    
    Dummy(uint32_t dummyId, float posX, float posY);
    
    void takeDamage(int damage);
    void resetHealth();
    bool checkCollision(const Player& player) const;
};

#endif // ENTITIES_H


#endif /* A39AF978_4111_486D_9C4F_505B7F57E811 */
