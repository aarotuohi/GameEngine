#include "GameClient.h"
#include "../shared/Protocol.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    // Initialize network subsystem
    if (!Protocol::initializeNetwork()) {
        std::cerr << "Failed to initialize network subsystem\n";
        return 1;
    }

    std::cout << "=== Game Client ===\n\n";
    
    // Get player name
    std::string playerName;
    std::cout << "Enter your name (or press Enter for default): ";
    std::getline(std::cin, playerName);
    if (playerName.empty()) {
        playerName = "Player";
    }
    
    // Get server address
    std::string serverHost;
    std::cout << "Enter server address (or press Enter for localhost): ";
    std::getline(std::cin, serverHost);
    if (serverHost.empty()) {
        serverHost = "127.0.0.1";
    }
    
    // Create and run client
    GameClient client(playerName);
    
    if (client.connect(serverHost)) {
        client.run();
    } else {
        std::cerr << "Could not connect to server. Make sure the server is running.\n";
    }
    
    Protocol::cleanupNetwork();
    return 0;
}
