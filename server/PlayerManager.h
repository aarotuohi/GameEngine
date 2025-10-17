#ifndef ACD3FC7E_BDF6_4A88_AF91_FA2DDDF2AEBF
#define ACD3FC7E_BDF6_4A88_AF91_FA2DDDF2AEBF
#ifndef PLAYERMANAGER_H
#define PLAYERMANAGER_H

#include <unordered_map>
#include <mutex>
#include <memory>
#include "GameState.h"
#include "../shared/Protocol.h"

#ifdef _WIN32
    #include <winsock2.h>
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
#endif

class PlayerManager {
private:
    GameState& gameState;
    std::unordered_map<uint32_t, SOCKET> playerTcpSockets;
    std::unordered_map<uint32_t, sockaddr_in> playerUdpAddresses;
    mutable std::mutex mutex;

public:
    explicit PlayerManager(GameState& state);
    ~PlayerManager();

    void addPlayer(uint32_t playerId, SOCKET tcpSocket);
    void removePlayer(uint32_t playerId);
    void registerUdpAddress(uint32_t playerId, const sockaddr_in& address);
    
    std::vector<sockaddr_in> getUdpAddresses();
    void broadcastTcp(uint8_t msgType, const std::vector<uint8_t>& data, uint32_t excludePlayerId = 0);
    void sendTcp(uint32_t playerId, uint8_t msgType, const std::vector<uint8_t>& data);
    
    int getPlayerCount() const;
};

#endif // PLAYERMANAGER_H


#endif /* ACD3FC7E_BDF6_4A88_AF91_FA2DDDF2AEBF */
