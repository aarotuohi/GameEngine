#ifndef F2591DFC_2882_477B_AA1D_1C2C500D9A7C
#define F2591DFC_2882_477B_AA1D_1C2C500D9A7C
#ifndef NETWORKHANDLER_H
#define NETWORKHANDLER_H

#include <thread>
#include <atomic>
#include "GameState.h"
#include "PlayerManager.h"
#include "../shared/Protocol.h"

class NetworkHandler {
private:
    GameState& gameState;
    PlayerManager& playerManager;
    
    SOCKET tcpSocket;
    SOCKET udpSocket;
    std::atomic<bool> running;
    
    std::thread tcpAcceptThread;
    std::thread udpReceiverThread;
    
    void acceptTcpConnections();
    void handleTcpClient(SOCKET clientSocket, sockaddr_in clientAddr);
    void receiveUdpMessages();
    void processUdpMessage(const uint8_t* data, size_t length, const sockaddr_in& senderAddr);

public:
    NetworkHandler(GameState& state, PlayerManager& manager);
    ~NetworkHandler();

    bool start(const std::string& host = "0.0.0.0");
    void broadcastUdpState();
    void stop();
};

#endif // NETWORKHANDLER_H


#endif /* F2591DFC_2882_477B_AA1D_1C2C500D9A7C */
