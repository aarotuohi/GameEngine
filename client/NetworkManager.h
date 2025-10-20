#ifndef A6514255_3B69_45E6_9CAB_ED37FD749420
#define A6514255_3B69_45E6_9CAB_ED37FD749420
#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <unordered_map>
#include <memory>
#include "../shared/Protocol.h"
#include "../shared/Entities.h"

class NetworkManager {
private:
    std::string playerName;
    uint32_t playerId;
    SOCKET tcpSocket;
    SOCKET udpSocket;
    sockaddr_in serverAddr;
    std::atomic<bool> running;
    
    std::thread tcpReceiverThread;
    std::thread udpReceiverThread;
    
    std::unordered_map<uint32_t, std::shared_ptr<Player>> players;
    mutable std::mutex playersMutex;
    
    void receiveTcpMessages();
    void receiveUdpMessages();
    void processTcpMessage(const Protocol::TCPMessage& message);
    void processUdpMessage(const Protocol::StateBroadcast& broadcast);

public:
    explicit NetworkManager(const std::string& name = "Player");
    ~NetworkManager();

    bool connect(const std::string& host = Config::SERVER_HOST);
    void disconnect();
    
    void sendPositionUpdate(float x, float y, float vx, float vy);
    void sendTcpMessage(uint8_t msgType, const std::vector<uint8_t>& data);
    
    uint32_t getPlayerId() const { return playerId; }
    std::unordered_map<uint32_t, std::shared_ptr<Player>> getPlayers();
    std::shared_ptr<Player> getPlayer(uint32_t pid);
};

#endif 


#endif 
