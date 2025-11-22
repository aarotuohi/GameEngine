#include "NetworkHandler.h"
#include "../shared/Config.h"
#include <iostream>
#include <cstring>
#include <cmath>

NetworkHandler::NetworkHandler(GameState& state, PlayerManager& manager)
    : gameState(state), playerManager(manager), 
      tcpSocket(INVALID_SOCKET), udpSocket(INVALID_SOCKET), running(false) {
}

NetworkHandler::~NetworkHandler() {
    stop();
}

bool NetworkHandler::start(const std::string& host) {
    running = true;
    
    // Setup TCP socket
    tcpSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (tcpSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create TCP socket\n";
        return false;
    }
    
    // Allow address reuse
    int reuse = 1;
    setsockopt(tcpSocket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&reuse), sizeof(reuse));
    
    sockaddr_in tcpAddr{};
    tcpAddr.sin_family = AF_INET;
    tcpAddr.sin_port = Protocol::hton(Config::TCP_PORT);
    tcpAddr.sin_addr.s_addr = INADDR_ANY;
    
    if (bind(tcpSocket, reinterpret_cast<sockaddr*>(&tcpAddr), sizeof(tcpAddr)) == SOCKET_ERROR) {
        std::cerr << "Failed to bind TCP socket\n";
        closesocket(tcpSocket);
        return false;
    }
    
    if (listen(tcpSocket, 5) == SOCKET_ERROR) {
        std::cerr << "Failed to listen on TCP socket\n";
        closesocket(tcpSocket);
        return false;
    }
    
    // Setup UDP socket
    udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (udpSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create UDP socket\n";
        closesocket(tcpSocket);
        return false;
    }
    
    sockaddr_in udpAddr{};
    udpAddr.sin_family = AF_INET;
    udpAddr.sin_port = Protocol::hton(Config::UDP_PORT);
    udpAddr.sin_addr.s_addr = INADDR_ANY;
    
    if (bind(udpSocket, reinterpret_cast<sockaddr*>(&udpAddr), sizeof(udpAddr)) == SOCKET_ERROR) {
        std::cerr << "Failed to bind UDP socket\n";
        closesocket(tcpSocket);
        closesocket(udpSocket);
        return false;
    }
    
    // Start threads
    tcpAcceptThread = std::thread(&NetworkHandler::acceptTcpConnections, this);
    udpReceiverThread = std::thread(&NetworkHandler::receiveUdpMessages, this);
    
    std::cout << "Server started - TCP: " << Config::TCP_PORT << ", UDP: " << Config::UDP_PORT << "\n";
    return true;
}

void NetworkHandler::acceptTcpConnections() {
    while (running) {
        sockaddr_in clientAddr{};
        socklen_t addrLen = sizeof(clientAddr);
        
        SOCKET clientSocket = accept(tcpSocket, reinterpret_cast<sockaddr*>(&clientAddr), &addrLen);
        if (clientSocket != INVALID_SOCKET) {
            std::cout << "New TCP connection\n";
            std::thread clientThread(&NetworkHandler::handleTcpClient, this, clientSocket, clientAddr);
            clientThread.detach();
        }
    }
}

void NetworkHandler::handleTcpClient(SOCKET clientSocket, sockaddr_in clientAddr) {
    uint32_t playerId = 0;
    std::vector<uint8_t> buffer;
    
    try {
        while (running) {
            uint8_t tempBuffer[Config::BUFFER_SIZE];
            int bytesReceived = recv(clientSocket, reinterpret_cast<char*>(tempBuffer), Config::BUFFER_SIZE, 0);
            
            if (bytesReceived <= 0) break;
            
            buffer.insert(buffer.end(), tempBuffer, tempBuffer + bytesReceived);
            
            // Process complete messages
            while (buffer.size() >= 5) {
                uint32_t msgLength;
                std::memcpy(&msgLength, buffer.data(), sizeof(msgLength));
                msgLength = Protocol::ntoh(msgLength);
                
                if (buffer.size() < 5 + msgLength) break;
                
                Protocol::TCPMessage message;
                if (Protocol::decodeTCPMessage(buffer, message)) {
                    // Process message
                    if (message.type == static_cast<uint8_t>(Config::MessageType::PLAYER_JOIN)) {
                        // Player joining
                        std::string playerName = "Player";
                        if (message.data.size() > 0) {
                            playerName.assign(message.data.begin(), message.data.end());
                        }
                        
                        playerId = gameState.addPlayer(playerName);
                        playerManager.addPlayer(playerId, clientSocket);
                        
                        // Send confirmation
                        std::vector<uint8_t> response;
                        uint32_t netPlayerId = Protocol::hton(playerId);
                        response.insert(response.end(), 
                                      reinterpret_cast<uint8_t*>(&netPlayerId),
                                      reinterpret_cast<uint8_t*>(&netPlayerId) + sizeof(netPlayerId));
                        
                        playerManager.sendTcp(playerId, 
                                            static_cast<uint8_t>(Config::MessageType::PLAYER_JOIN), 
                                            response);
                    }
                    
                    buffer.erase(buffer.begin(), buffer.begin() + 5 + msgLength);
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error handling client: " << e.what() << "\n";
    }
    
    if (playerId != 0) {
        playerManager.removePlayer(playerId);
    }
    closesocket(clientSocket);
}

void NetworkHandler::receiveUdpMessages() {
    while (running) {
        uint8_t buffer[Config::BUFFER_SIZE];
        sockaddr_in senderAddr{};
        socklen_t addrLen = sizeof(senderAddr);
        
        int bytesReceived = recvfrom(udpSocket, reinterpret_cast<char*>(buffer), Config::BUFFER_SIZE,
                                    0, reinterpret_cast<sockaddr*>(&senderAddr), &addrLen);
        
        if (bytesReceived > 0) {
            processUdpMessage(buffer, bytesReceived, senderAddr);
        }
    }
}

void NetworkHandler::processUdpMessage(const uint8_t* data, size_t length, const sockaddr_in& senderAddr) {
    if (length < 1) return;
    
    // Check update type
    uint8_t updateType = data[0];
    const uint8_t* payload = data + 1;
    size_t payloadLength = length - 1;
    
    if (updateType == static_cast<uint8_t>(Config::UpdateType::POSITION)) {
        Protocol::PositionUpdate update;
        if (Protocol::decodePositionUpdate(payload, payloadLength, update)) {
            gameState.setPlayerPosition(update.playerId, update.x, update.y);
            gameState.updatePlayerVelocity(update.playerId, update.vx, update.vy);
            playerManager.registerUdpAddress(update.playerId, senderAddr);
        }
    } else if (updateType == static_cast<uint8_t>(Config::UpdateType::ABILITY_USE)) {
        Protocol::AbilityUse ability;
        if (Protocol::decodeAbilityUse(payload, payloadLength, ability)) {
            playerManager.registerUdpAddress(ability.playerId, senderAddr);

            if (ability.abilityType == 1) {
                auto player = gameState.getPlayer(ability.playerId);
                if (player && player->canUseQ()) {
                    float dirX = ability.targetX;
                    float dirY = ability.targetY;
                    float len = std::sqrt(dirX * dirX + dirY * dirY);
                    if (len > 0.0001f) { dirX /= len; dirY /= len; }

                    bool isTornado = (player->qStacks >= 2);
                    if (isTornado) {
                        const int tornadoDamage = 40;
                        gameState.createQProjectile(ability.playerId, player->x, player->y,
                                                    dirX, dirY, true, tornadoDamage);
                        player->qStacks = 0;
                    } else {
                        gameState.processQSwordSwing(
                            ability.playerId,
                            player->x, player->y,
                            dirX, dirY,
                            Config::Q_SWORD_ARC_DEGREES,
                            Config::Q_SWORD_RANGE,
                            Config::Q_SWORD_DAMAGE
                        );
                    }

                    player->lastQTime = std::chrono::steady_clock::now();
                    player->activeAbility = SamuraiAbility::Q_STEEL_TEMPEST;
                }
            } else if (ability.abilityType == 2) {
                auto player = gameState.getPlayer(ability.playerId);
                if (player && player->canUseW()) {
                    player->useW();
                }
            } else if (ability.abilityType == 3) {
              
                auto player = gameState.getPlayer(ability.playerId);
                if (player && player->canUseE()) {
                    player->useE();
                    
                    gameState.processEShockwave(ability.playerId, player->x, player->y, 
                                                Config::E_SHOCKWAVE_RADIUS, Config::E_SHOCKWAVE_DAMAGE);
                    std::cout << "Player " << ability.playerId << " cast E shockwave at (" 
                              << player->x << ", " << player->y << ")\n";
                }
            } else if (ability.abilityType == 4) {
                auto player = gameState.getPlayer(ability.playerId);
                if (player && player->canUseR()) {
                    player->useR();
                    gameState.createRTornadoes(ability.playerId);
                }
            }
        }
    } else {

        Protocol::PositionUpdate update;
        if (Protocol::decodePositionUpdate(data, length, update)) {
            gameState.setPlayerPosition(update.playerId, update.x, update.y);
            gameState.updatePlayerVelocity(update.playerId, update.vx, update.vy);
            playerManager.registerUdpAddress(update.playerId, senderAddr);
        }
    }
}

void NetworkHandler::broadcastUdpState() {
    if (!running) return;
    
    auto players = gameState.getPlayersForBroadcast();
    if (players.empty()) return;
    
    Protocol::StateBroadcast broadcast;
    broadcast.numPlayers = static_cast<uint32_t>(players.size());
    broadcast.players = players;
    
   
    auto enemies = gameState.getAllEnemies();
    broadcast.numEnemies = static_cast<uint32_t>(enemies.size());
    for (const auto& [enemyId, enemy] : enemies) {
        Protocol::EnemyState state;
        state.id = enemy->id;
        state.x = enemy->x;
        state.y = enemy->y;
        state.vx = enemy->vx;
        state.vy = enemy->vy;
        state.health = enemy->health;
        state.isAlive = enemy->isAlive;
        state.targetPlayerId = enemy->targetPlayerId;
        state.isBoss = enemy->isBoss;
        state.size = enemy->size;
        broadcast.enemies.push_back(state);
    }
    
    // Add projectiles
    auto projectiles = gameState.getAllProjectiles();
    for (const auto& [projId, proj] : projectiles) {
        if (!proj->active) continue; 
        
        Protocol::ProjectileState state;
        state.id = proj->id;
        state.x = proj->x;
        state.y = proj->y;
        state.vx = proj->vx;
        state.vy = proj->vy;
        state.isTornado = proj->isTornado;
        state.isEnemyProjectile = proj->isEnemyProjectile;
        state.ownerId = proj->ownerId;
        broadcast.projectiles.push_back(state);
    }
    broadcast.numProjectiles = static_cast<uint32_t>(broadcast.projectiles.size());
    
    auto rTornadoes = gameState.getAllRTornadoes();
    broadcast.numRTornadoes = static_cast<uint32_t>(rTornadoes.size());
    
   
    
    for (const auto& [tornadoId, tornado] : rTornadoes) {
        auto owner = gameState.getPlayer(tornado->ownerId);
        if (owner) {
            Protocol::RTornadoState state;
            state.id = tornado->id;
            state.ownerId = tornado->ownerId;
            
            // Calculate tornado position with NaN protection
            float tornadoX = owner->x + std::cos(tornado->angle) * Config::R_ORBIT_RADIUS;
            float tornadoY = owner->y + std::sin(tornado->angle) * Config::R_ORBIT_RADIUS;
            
            // Protect against NaN positions being sent to clients
            if (std::isnan(tornadoX) || std::isnan(tornadoY) || std::isinf(tornadoX) || std::isinf(tornadoY)) {
                std::cout << "WARNING: RTornado " << tornado->id << " has invalid position! Skipping...\n";
                continue;
            }
            
            state.x = tornadoX;
            state.y = tornadoY;
            broadcast.rTornadoes.push_back(state);
            
        }
    }
    
    
    broadcast.currentWave = gameState.getCurrentWave();
    
    auto data = Protocol::encodeStateBroadcast(broadcast);
    auto addresses = playerManager.getUdpAddresses();
    
    for (const auto& addr : addresses) {
        sendto(udpSocket, reinterpret_cast<const char*>(data.data()), static_cast<int>(data.size()),
               0, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr));
    }
}

void NetworkHandler::stop() {
    running = false;
    
    if (tcpSocket != INVALID_SOCKET) {
        closesocket(tcpSocket);
        tcpSocket = INVALID_SOCKET;
    }
    
    if (udpSocket != INVALID_SOCKET) {
        closesocket(udpSocket);
        udpSocket = INVALID_SOCKET;
    }
    
    if (tcpAcceptThread.joinable()) tcpAcceptThread.join();
    if (udpReceiverThread.joinable()) udpReceiverThread.join();
}
