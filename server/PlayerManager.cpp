#include "PlayerManager.h"
#include <iostream>

PlayerManager::PlayerManager(GameState& state) : gameState(state) {
}

PlayerManager::~PlayerManager() {
}

void PlayerManager::addPlayer(uint32_t playerId, SOCKET tcpSocket) {
    std::lock_guard<std::mutex> lock(mutex);
    playerTcpSockets[playerId] = tcpSocket;
    std::cout << "Player " << playerId << " TCP connection registered\n";
}

void PlayerManager::removePlayer(uint32_t playerId) {
    std::lock_guard<std::mutex> lock(mutex);
    
    auto tcpIt = playerTcpSockets.find(playerId);
    if (tcpIt != playerTcpSockets.end()) {
        playerTcpSockets.erase(tcpIt);
    }
    
    auto udpIt = playerUdpAddresses.find(playerId);
    if (udpIt != playerUdpAddresses.end()) {
        playerUdpAddresses.erase(udpIt);
    }
    
    gameState.removePlayer(playerId);
    
    std::cout << "Player " << playerId << " disconnected\n";
}

void PlayerManager::registerUdpAddress(uint32_t playerId, const sockaddr_in& address) {
    std::lock_guard<std::mutex> lock(mutex);
    playerUdpAddresses[playerId] = address;
}

std::vector<sockaddr_in> PlayerManager::getUdpAddresses() {
    std::lock_guard<std::mutex> lock(mutex);
    std::vector<sockaddr_in> addresses;
    for (const auto& [playerId, addr] : playerUdpAddresses) {
        addresses.push_back(addr);
    }
    return addresses;
}

void PlayerManager::broadcastTcp(uint8_t msgType, const std::vector<uint8_t>& data, uint32_t excludePlayerId) {
    std::lock_guard<std::mutex> lock(mutex);
    auto message = Protocol::encodeTCPMessage(msgType, data);
    
    for (const auto& [playerId, socket] : playerTcpSockets) {
        if (playerId != excludePlayerId) {
            int result = send(socket, reinterpret_cast<const char*>(message.data()), 
                            static_cast<int>(message.size()), 0);
            if (result == SOCKET_ERROR) {
                std::cerr << "Error broadcasting to player " << playerId << "\n";
            }
        }
    }
}

void PlayerManager::sendTcp(uint32_t playerId, uint8_t msgType, const std::vector<uint8_t>& data) {
    std::lock_guard<std::mutex> lock(mutex);
    auto it = playerTcpSockets.find(playerId);
    if (it != playerTcpSockets.end()) {
        auto message = Protocol::encodeTCPMessage(msgType, data);
        int result = send(it->second, reinterpret_cast<const char*>(message.data()),
                         static_cast<int>(message.size()), 0);
        if (result == SOCKET_ERROR) {
            std::cerr << "Error sending to player " << playerId << "\n";
        }
    }
}

int PlayerManager::getPlayerCount() const {
    std::lock_guard<std::mutex> lock(mutex);
    return static_cast<int>(playerTcpSockets.size());
}
