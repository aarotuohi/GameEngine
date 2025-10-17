#include "GameServer.h"
#include "../shared/Protocol.h"
#include <iostream>
#include <csignal>

GameServer* g_server = nullptr;

void signalHandler(int signal) {
    if (g_server) {
        g_server->stop();
    }
}

int main() {
    // Initialize network subsystem
    if (!Protocol::initializeNetwork()) {
        std::cerr << "Failed to initialize network subsystem\n";
        return 1;
    }

    // Setup signal handler for graceful shutdown
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    // Create and start server
    GameServer server;
    g_server = &server;

    if (server.start()) {
        server.run();
    }

    g_server = nullptr;
    Protocol::cleanupNetwork();
    
    return 0;
}
