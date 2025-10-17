#include "NetworkManager.h"
#include "../shared/Config.h"
#include <iostream>
#include <cstring>

NetworkManager::NetworkManager(const std::string& name)
    : playerName(name), playerId(0), tcpSocket(INVALID_SOCKET), 
      udpSocket(INVALID_SOCKET), running(false) {
}

NetworkManager::~NetworkManager() {
    disconnect();
}

bool NetworkManager::connect(const std::string& host) {
    // Create TCP socket
    tcpSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (tcpSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create TCP socket\n";
        return false;
    }
    
    // Setup server address
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = Protocol::hton(Config::TCP_PORT);
    #ifdef _WIN32
        serverAddr.sin_addr.s_addr = inet_addr(host.c_str());
    #else
        inet_pton(AF_INET, host.c_str(), &serverAddr.sin_addr);
    #endif
    
    // Connect TCP
    if (::connect(tcpSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Failed to connect to server\n";
        closesocket(tcpSocket);
        return false;
    }
    
    std::cout << "Connected to server via TCP\n";
    
    // Create UDP socket
    udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (udpSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create UDP socket\n";
        closesocket(tcpSocket);
        return false;
    }
    
    // Setup UDP server address
    sockaddr_in udpAddr = serverAddr;
    udpAddr.sin_port = Protocol::hton(Config::UDP_PORT);
    
    std::cout << "UDP socket ready\n";
    
    running = true;
    
    // Start receiver threads
    tcpReceiverThread = std::thread(&NetworkManager::receiveTcpMessages, this);
    udpReceiverThread = std::thread(&NetworkManager::receiveUdpMessages, this);
    
    // Send join request
    std::vector<uint8_t> nameData(playerName.begin(), playerName.end());
    sendTcpMessage(static_cast<uint8_t>(Config::MessageType::PLAYER_JOIN), nameData);
    
    // Wait a bit for player ID assignment
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    return true;
}

void NetworkManager::receiveTcpMessages() {
    std::vector<uint8_t> buffer;
    
    while (running) {
        uint8_t tempBuffer[Config::BUFFER_SIZE];
        int bytesReceived = recv(tcpSocket, reinterpret_cast<char*>(tempBuffer), Config::BUFFER_SIZE, 0);
        
        if (bytesReceived <= 0) {
            std::cout << "Server disconnected\n";
            running = false;
            break;
        }
        
        buffer.insert(buffer.end(), tempBuffer, tempBuffer + bytesReceived);
        
        // Process complete messages
        while (buffer.size() >= 5) {
            Protocol::TCPMessage message;
            if (Protocol::decodeTCPMessage(buffer, message)) {
                processTcpMessage(message);
                buffer.erase(buffer.begin(), buffer.begin() + 5 + message.dataLength);
            } else {
                break;
            }
        }
    }
}

void NetworkManager::receiveUdpMessages() {
    sockaddr_in udpServerAddr{};
    udpServerAddr.sin_family = AF_INET;
    udpServerAddr.sin_port = Protocol::hton(Config::UDP_PORT);
    udpServerAddr.sin_addr = serverAddr.sin_addr;
    
    while (running) {
        uint8_t buffer[Config::BUFFER_SIZE];
        sockaddr_in senderAddr{};
        socklen_t addrLen = sizeof(senderAddr);
        
        int bytesReceived = recvfrom(udpSocket, reinterpret_cast<char*>(buffer), Config::BUFFER_SIZE,
                                     0, reinterpret_cast<sockaddr*>(&senderAddr), &addrLen);
        
        if (bytesReceived > 0) {
            Protocol::StateBroadcast broadcast;
            if (Protocol::decodeStateBroadcast(buffer, bytesReceived, broadcast)) {
                processUdpMessage(broadcast);
            }
        }
    }
}

void NetworkManager::processTcpMessage(const Protocol::TCPMessage& message) {
    if (message.type == static_cast<uint8_t>(Config::MessageType::PLAYER_JOIN)) {
        if (message.data.size() >= sizeof(uint32_t)) {
            std::memcpy(&playerId, message.data.data(), sizeof(playerId));
            playerId = Protocol::ntoh(playerId);
            std::cout << "Assigned player ID: " << playerId << "\n";
        }
    }
}

void NetworkManager::processUdpMessage(const Protocol::StateBroadcast& broadcast) {
    std::lock_guard<std::mutex> lock(playersMutex);
    
    for (const auto& state : broadcast.players) {
        auto it = players.find(state.id);
        if (it != players.end()) {
            // Update existing player
            it->second->setPosition(state.x, state.y);
            it->second->updateVelocity(state.vx, state.vy);
        } else {
            // Add new player
            auto player = std::make_shared<Player>(state.id, state.x, state.y);
            player->updateVelocity(state.vx, state.vy);
            players[state.id] = player;
        }
    }
}

void NetworkManager::sendPositionUpdate(float x, float y, float vx, float vy) {
    if (playerId == 0) return;
    
    Protocol::PositionUpdate update;
    update.playerId = playerId;
    update.x = x;
    update.y = y;
    update.vx = vx;
    update.vy = vy;
    update.timestamp = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    
    auto data = Protocol::encodePositionUpdate(update);
    
    sockaddr_in udpAddr = serverAddr;
    udpAddr.sin_port = Protocol::hton(Config::UDP_PORT);
    
    sendto(udpSocket, reinterpret_cast<const char*>(data.data()), static_cast<int>(data.size()),
           0, reinterpret_cast<const sockaddr*>(&udpAddr), sizeof(udpAddr));
}

void NetworkManager::sendTcpMessage(uint8_t msgType, const std::vector<uint8_t>& data) {
    auto message = Protocol::encodeTCPMessage(msgType, data);
    send(tcpSocket, reinterpret_cast<const char*>(message.data()), 
         static_cast<int>(message.size()), 0);
}

std::unordered_map<uint32_t, std::shared_ptr<Player>> NetworkManager::getPlayers() {
    std::lock_guard<std::mutex> lock(playersMutex);
    return players;
}

std::shared_ptr<Player> NetworkManager::getPlayer(uint32_t pid) {
    std::lock_guard<std::mutex> lock(playersMutex);
    auto it = players.find(pid);
    return (it != players.end()) ? it->second : nullptr;
}

void NetworkManager::disconnect() {
    running = false;
    
    if (tcpSocket != INVALID_SOCKET) {
        sendTcpMessage(static_cast<uint8_t>(Config::MessageType::PLAYER_LEAVE), {});
        closesocket(tcpSocket);
        tcpSocket = INVALID_SOCKET;
    }
    
    if (udpSocket != INVALID_SOCKET) {
        closesocket(udpSocket);
        udpSocket = INVALID_SOCKET;
    }
    
    if (tcpReceiverThread.joinable()) tcpReceiverThread.join();
    if (udpReceiverThread.joinable()) udpReceiverThread.join();
    
    std::cout << "Disconnected from server\n";
}
