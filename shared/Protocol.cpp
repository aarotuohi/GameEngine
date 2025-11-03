#include "Protocol.h"
#include <cstring>
#include <chrono>

namespace Protocol {

    // Network initialization
    bool initializeNetwork() {
        #ifdef _WIN32
        WSADATA wsaData;
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        return result == 0;
        #else
        
        #endif
    }

    void cleanupNetwork() {
        #ifdef _WIN32
        WSACleanup();
        #endif
    }

    // Byte order conversion
    template<>
    uint16_t ntoh(uint16_t value) {
        return ntohs(value);
    }

    template<>
    uint32_t ntoh(uint32_t value) {
        return ntohl(value);
    }

    template<>
    uint16_t hton(uint16_t value) {
        return htons(value);
    }

    template<>
    uint32_t hton(uint32_t value) {
        return htonl(value);
    }

    // TCP Message encoding
    std::vector<uint8_t> encodeTCPMessage(uint8_t type, const std::vector<uint8_t>& data) {
        std::vector<uint8_t> buffer;
        uint32_t length = hton(static_cast<uint32_t>(data.size()));
        
        // Add length (4 bytes)
        buffer.insert(buffer.end(), 
                     reinterpret_cast<uint8_t*>(&length),
                     reinterpret_cast<uint8_t*>(&length) + sizeof(length));
        
        // Add type (1 byte)
        buffer.push_back(type);
        
        // Add data
        buffer.insert(buffer.end(), data.begin(), data.end());
        
        return buffer;
    }

    // TCP Message decoding
    bool decodeTCPMessage(const std::vector<uint8_t>& buffer, TCPMessage& message) {
        if (buffer.size() < 5) return false; 
        
        // Extract length
        uint32_t length;
        std::memcpy(&length, buffer.data(), sizeof(length));
        length = ntoh(length);
        
        if (buffer.size() < 5 + length) return false;
        
        // Extract type
        message.type = buffer[4];
        
        // Extract data
        message.dataLength = length;
        message.data.assign(buffer.begin() + 5, buffer.begin() + 5 + length);
        
        return true;
    }

    // Position update encoding
    std::vector<uint8_t> encodePositionUpdate(const PositionUpdate& update) {
        std::vector<uint8_t> buffer(sizeof(PositionUpdate));
        
        // Convert to network byte order
        PositionUpdate netUpdate = update;
        netUpdate.playerId = hton(update.playerId);
        
        // Copy to buffer
        std::memcpy(buffer.data(), &netUpdate, sizeof(PositionUpdate));
        
        return buffer;
    }

    // Position update decoding
    bool decodePositionUpdate(const uint8_t* data, size_t length, PositionUpdate& update) {
        if (length < sizeof(PositionUpdate)) return false;
        
        std::memcpy(&update, data, sizeof(PositionUpdate));
        update.playerId = ntoh(update.playerId);
        
        return true;
    }

    // Ability use encoding
    std::vector<uint8_t> encodeAbilityUse(const AbilityUse& ability) {
        std::vector<uint8_t> buffer(sizeof(AbilityUse));
        
        // Convert to network byte order
        AbilityUse netAbility = ability;
        netAbility.playerId = hton(ability.playerId);
        
        // Copy to buffer
        std::memcpy(buffer.data(), &netAbility, sizeof(AbilityUse));
        
        return buffer;
    }

    // Ability use decoding
    bool decodeAbilityUse(const uint8_t* data, size_t length, AbilityUse& ability) {
        if (length < sizeof(AbilityUse)) return false;
        
        std::memcpy(&ability, data, sizeof(AbilityUse));
        ability.playerId = ntoh(ability.playerId);
        
        return true;
    }


    std::vector<uint8_t> encodeStateBroadcast(const StateBroadcast& state) {
        std::vector<uint8_t> buffer;
        
        // Add number of players
        uint32_t numPlayers = hton(state.numPlayers);
        buffer.insert(buffer.end(),
                     reinterpret_cast<uint8_t*>(&numPlayers),
                     reinterpret_cast<uint8_t*>(&numPlayers) + sizeof(numPlayers));
        
        // Add each player state
        for (const auto& player : state.players) {
            PlayerState netPlayer = player;
            netPlayer.id = hton(player.id);
            
            buffer.insert(buffer.end(),
                         reinterpret_cast<const uint8_t*>(&netPlayer),
                         reinterpret_cast<const uint8_t*>(&netPlayer) + sizeof(PlayerState));
        }
        
      
        uint32_t numEnemies = hton(state.numEnemies);
        buffer.insert(buffer.end(),
                     reinterpret_cast<uint8_t*>(&numEnemies),
                     reinterpret_cast<uint8_t*>(&numEnemies) + sizeof(numEnemies));
        
        
        for (const auto& enemy : state.enemies) {
            EnemyState netEnemy = enemy;
            netEnemy.id = hton(enemy.id);
            netEnemy.health = hton(enemy.health);
            netEnemy.targetPlayerId = hton(enemy.targetPlayerId);
            
            buffer.insert(buffer.end(),
                         reinterpret_cast<const uint8_t*>(&netEnemy),
                         reinterpret_cast<const uint8_t*>(&netEnemy) + sizeof(EnemyState));
        }
        
        // Add number of projectiles
        uint32_t numProjectiles = hton(state.numProjectiles);
        buffer.insert(buffer.end(),
                     reinterpret_cast<uint8_t*>(&numProjectiles),
                     reinterpret_cast<uint8_t*>(&numProjectiles) + sizeof(numProjectiles));
        
        // Add each projectile state
        for (const auto& proj : state.projectiles) {
            ProjectileState netProj = proj;
            netProj.id = hton(proj.id);
            netProj.ownerId = hton(proj.ownerId);
            
            buffer.insert(buffer.end(),
                         reinterpret_cast<const uint8_t*>(&netProj),
                         reinterpret_cast<const uint8_t*>(&netProj) + sizeof(ProjectileState));
        }

        uint32_t netNumRTornadoes = hton(state.numRTornadoes);
        buffer.insert(buffer.end(),
                     reinterpret_cast<const uint8_t*>(&netNumRTornadoes),
                     reinterpret_cast<const uint8_t*>(&netNumRTornadoes) + sizeof(uint32_t));
        
        for (const auto& tornado : state.rTornadoes) {
            RTornadoState netTornado = tornado;
            netTornado.id = hton(tornado.id);
            netTornado.ownerId = hton(tornado.ownerId);
            
            buffer.insert(buffer.end(),
                         reinterpret_cast<const uint8_t*>(&netTornado),
                         reinterpret_cast<const uint8_t*>(&netTornado) + sizeof(RTornadoState));
        }
        
        return buffer;
    }

    // State broadcast decoding
    bool decodeStateBroadcast(const uint8_t* data, size_t length, StateBroadcast& state) {
        if (length < sizeof(uint32_t)) return false;
        
        std::memcpy(&state.numPlayers, data, sizeof(uint32_t));
        state.numPlayers = ntoh(state.numPlayers);
        
        size_t expectedSize = sizeof(uint32_t) + state.numPlayers * sizeof(PlayerState);
        if (length < expectedSize) return false;
        
        
        state.players.clear();
        const uint8_t* ptr = data + sizeof(uint32_t);
        
        for (uint32_t i = 0; i < state.numPlayers; ++i) {
            PlayerState player;
            std::memcpy(&player, ptr, sizeof(PlayerState));
            player.id = ntoh(player.id);
            state.players.push_back(player);
            ptr += sizeof(PlayerState);
        }
        
        
        if (length >= expectedSize + sizeof(uint32_t)) {
            std::memcpy(&state.numEnemies, ptr, sizeof(uint32_t));
            state.numEnemies = ntoh(state.numEnemies);
            ptr += sizeof(uint32_t);
            
            expectedSize += sizeof(uint32_t) + state.numEnemies * sizeof(EnemyState);
            if (length >= expectedSize) {
              
                state.enemies.clear();
                for (uint32_t i = 0; i < state.numEnemies; ++i) {
                    EnemyState enemy;
                    std::memcpy(&enemy, ptr, sizeof(EnemyState));
                    enemy.id = ntoh(enemy.id);
                    enemy.health = ntoh(enemy.health);
                    enemy.targetPlayerId = ntoh(enemy.targetPlayerId);
                    state.enemies.push_back(enemy);
                    ptr += sizeof(EnemyState);
                }
            }
        } else {
            state.numEnemies = 0;
        }
        
        // Extract number of projectiles if data available
        if (length >= expectedSize + sizeof(uint32_t)) {
            std::memcpy(&state.numProjectiles, ptr, sizeof(uint32_t));
            state.numProjectiles = ntoh(state.numProjectiles);
            ptr += sizeof(uint32_t);
            
            expectedSize += sizeof(uint32_t) + state.numProjectiles * sizeof(ProjectileState);
            if (length >= expectedSize) {
             
                state.projectiles.clear();
                for (uint32_t i = 0; i < state.numProjectiles; ++i) {
                    ProjectileState proj;
                    std::memcpy(&proj, ptr, sizeof(ProjectileState));
                    proj.id = ntoh(proj.id);
                    proj.ownerId = ntoh(proj.ownerId);
                    state.projectiles.push_back(proj);
                    ptr += sizeof(ProjectileState);
                }
            }
        } else {
            state.numProjectiles = 0;
        }
        
        if (length >= expectedSize + sizeof(uint32_t)) {
            std::memcpy(&state.numRTornadoes, ptr, sizeof(uint32_t));
            state.numRTornadoes = ntoh(state.numRTornadoes);
            ptr += sizeof(uint32_t);
            
            expectedSize += sizeof(uint32_t) + state.numRTornadoes * sizeof(RTornadoState);
            if (length >= expectedSize) {
                
                state.rTornadoes.clear();
                for (uint32_t i = 0; i < state.numRTornadoes; ++i) {
                    RTornadoState tornado;
                    std::memcpy(&tornado, ptr, sizeof(RTornadoState));
                    tornado.id = ntoh(tornado.id);
                    tornado.ownerId = ntoh(tornado.ownerId);
                    state.rTornadoes.push_back(tornado);
                    ptr += sizeof(RTornadoState);
                }
            }
        } else {
            state.numRTornadoes = 0;
        }
        
        return true;
    }
}
