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

class GameState {
private:
    std::unordered_map<uint32_t, std::shared_ptr<Player>> players;
    std::unordered_map<uint32_t, std::shared_ptr<Projectile>> projectiles;
    uint32_t nextPlayerId;
    uint32_t nextProjectileId;
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
    
    // Game loop
    void update(float dt);
    
    // State retrieval
    std::vector<Protocol::PlayerState> getPlayersForBroadcast();
    std::unordered_map<uint32_t, std::shared_ptr<Player>> getAllPlayers();
    
    // Control
    void setRunning(bool run) { running = run; }
    bool isRunning() const { return running; }
};

#endif // GAMESTATE_H


#endif /* F535EB1D_AFF1_4BD2_9C72_FEEC2272F2E9 */
