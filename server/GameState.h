#ifndef F535EB1D_AFF1_4BD2_9C72_FEEC2272F2E9
#define F535EB1D_AFF1_4BD2_9C72_FEEC2272F2E9
#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <unordered_map>
#include <memory>
#include <mutex>
#include <vector>
#include "../shared/Entities.h"
#include "../shared/Protocol.h"

struct RTornado {
    uint32_t id;
    uint32_t ownerId;
    float angle;  
    int tornadoIndex;  
    std::chrono::steady_clock::time_point lastHitTime;
    
    RTornado(uint32_t tid, uint32_t owner, float ang, int idx)
        : id(tid), ownerId(owner), angle(ang), tornadoIndex(idx),
          lastHitTime(std::chrono::steady_clock::now()) {}
};

class GameState {
private:
    std::unordered_map<uint32_t, std::shared_ptr<Player>> players;
    std::unordered_map<uint32_t, std::shared_ptr<Projectile>> projectiles;
    std::unordered_map<uint32_t, std::shared_ptr<Enemy>> enemies;
    std::unordered_map<uint32_t, std::shared_ptr<RTornado>> rTornadoes;
    uint32_t nextPlayerId;
    uint32_t nextProjectileId;
    uint32_t nextEnemyId;
    uint32_t nextRTornadoId;
    uint32_t taggedPlayerId;
    mutable std::mutex mutex;
    bool running;

public:
    GameState();
    ~GameState();

    // Player management
    uint32_t addPlayer(const std::string& name = "Player");
    void removePlayer(uint32_t playerId);
    std::shared_ptr<Player> getPlayer(uint32_t playerId);
    
    // Player updates
    void updatePlayerPosition(uint32_t playerId, float dx, float dy, float dt);
    void updatePlayerVelocity(uint32_t playerId, float vx, float vy);
    void setPlayerPosition(uint32_t playerId, float x, float y);
    
    // Projectile management
    uint32_t createProjectile(uint32_t ownerId, float x, float y, float vx, float vy);
    uint32_t createQProjectile(uint32_t ownerId, float x, float y, float dirX, float dirY, bool isTornado, int damage);

    bool processQSwordSwing(uint32_t ownerId, float originX, float originY, float dirX, float dirY,
                            float arcDegrees, float range, int damage);

    void processEDashDamage(uint32_t ownerId, float endX, float endY, float radius, int damage);
    
  
    void createRTornadoes(uint32_t ownerId);
    void updateRTornadoes(float dt);
    void removeRTornadoes(uint32_t ownerId);
    
   
    uint32_t spawnEnemy(float x, float y, uint32_t targetPlayerId = 0);
    std::shared_ptr<Enemy> getEnemy(uint32_t enemyId);
    void damageEnemy(uint32_t enemyId, int damage);
    void updateEnemyShooting(float dt);
    
    // Game loop
    void update(float dt);
    
    // State retrieval
    std::vector<Protocol::PlayerState> getPlayersForBroadcast();
    std::unordered_map<uint32_t, std::shared_ptr<Player>> getAllPlayers();
    std::unordered_map<uint32_t, std::shared_ptr<Enemy>> getAllEnemies();
    std::unordered_map<uint32_t, std::shared_ptr<Projectile>> getAllProjectiles();
    std::unordered_map<uint32_t, std::shared_ptr<RTornado>> getAllRTornadoes();
    
    // Control
    void setRunning(bool run) { running = run; }
    bool isRunning() const { return running; }
};

#endif // GAMESTATE_H


#endif /* F535EB1D_AFF1_4BD2_9C72_FEEC2272F2E9 */
