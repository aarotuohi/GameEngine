#ifndef C622334A_2331_46F3_9851_AA9DCC657D3A
#define C622334A_2331_46F3_9851_AA9DCC657D3A
#ifndef GAMESERVER_H
#define GAMESERVER_H

#include <thread>
#include <atomic>
#include <memory>
#include "GameState.h"
#include "PlayerManager.h"
#include "NetworkHandler.h"

class GameServer {
private:
    std::unique_ptr<GameState> gameState;
    std::unique_ptr<PlayerManager> playerManager;
    std::unique_ptr<NetworkHandler> networkHandler;
    
    std::atomic<bool> running;
    std::thread gameLoopThread;
    std::thread udpBroadcastThread;
    
    void gameLoop();
    void udpBroadcastLoop();

public:
    GameServer();
    ~GameServer();

    bool start();
    void stop();
    void run();
};

#endif // GAMESERVER_H


#endif /* C622334A_2331_46F3_9851_AA9DCC657D3A */
