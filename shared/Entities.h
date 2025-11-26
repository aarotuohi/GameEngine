#ifndef A39AF978_4111_486D_9C4F_505B7F57E811
#define A39AF978_4111_486D_9C4F_505B7F57E811
#ifndef ENTITIES_H
#define ENTITIES_H

#include <string>
#include <cstdint>
#include <chrono>
#include "Config.h"

// Forward declarations
class Enemy;


enum class SamuraiAbility {
    NONE = 0,
    Q_STEEL_TEMPEST = 1,  
    W_WIND_WALL = 2,        
    E_SWEEPING_BLADE = 3,   
    R_LAST_BREATH = 4       
};


class Player {
public:
    uint32_t id;
    std::string name;
    float x, y;     
    float vx, vy;      
    float size;
    float speed;
    bool isTagged;
    int score;
    std::chrono::steady_clock::time_point lastUpdate;


    int health;
    int maxHealth;
    float rotation;      
    bool isAlive;
    int kills;
    int level;
    int attackDamage;
    SamuraiAbility activeAbility;
    std::chrono::steady_clock::time_point lastQTime;
    std::chrono::steady_clock::time_point lastWTime;
    std::chrono::steady_clock::time_point lastETime;
    std::chrono::steady_clock::time_point lastRTime;
    int qStacks;         
    
    
    std::chrono::steady_clock::time_point eShockwaveTime;
    

    bool hasWindWall;
    std::chrono::steady_clock::time_point windWallStartTime;
    float windWallRadius; 
    
    
    bool hasRTornadoes;
    std::chrono::steady_clock::time_point rTornadoesStartTime;
    float rTornadoAngle; 
   
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
    void useE();
    void useR();
    void takeDamage(int damage);
    void respawn(float spawnX, float spawnY);
    
    // Wind Wall methods
    void updateWindWall(float dt);
    bool isProjectileBlockedByWindWall(float projX, float projY) const;
    
    void updateRTornadoes(float dt);
};


class Projectile {
public:
    uint32_t id;
    float x, y;          
    float vx, vy;        
    uint32_t ownerId;
    float size;
    float speed;
    bool active;
    bool isTornado;     
    bool isEnemyProjectile;  
    int damage;      

    Projectile(uint32_t projId, float posX, float posY, float velX, float velY, uint32_t owner, bool tornado = false, int dmg = 20, bool enemyProj = false);
    
    void update(float dt);
    bool checkCollision(const Player& player) const;
    bool checkCollisionWithEnemy(const Enemy& enemy) const;
};


class Enemy {
public:
    uint32_t id;
    float x, y;          
    float vx, vy;       
    float size;
    float speed;        
    int health;
    int maxHealth;
    bool isAlive;
    uint32_t targetPlayerId; 
    uint32_t lastDamagedBy;
    float rotation;          
    std::chrono::steady_clock::time_point lastHitTime;
    std::chrono::steady_clock::time_point spawnTime;
    std::chrono::steady_clock::time_point lastShootTime;
    bool isBoss;
    int killValue;
    bool isDragon;
    
    Enemy(uint32_t enemyId, float posX, float posY, uint32_t targetPlayer = 0, bool boss = false);
    
    void takeDamage(int damage, uint32_t damagerId = 0);
    void setTarget(uint32_t playerId);
    void moveTowards(float targetX, float targetY, float dt);
    void update(float dt);
    bool checkCollision(const Player& player) const;
    bool canShoot() const;
    void shoot();
};

#endif // ENTITIES_H


#endif /* A39AF978_4111_486D_9C4F_505B7F57E811 */
