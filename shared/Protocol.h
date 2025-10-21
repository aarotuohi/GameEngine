#ifndef A43EA098_68BA_4EED_8976_AFEACF663129
#define A43EA098_68BA_4EED_8976_AFEACF663129
#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <cstdint>
#include <vector>
#include <string>
#include <cstring>

// Cross-platform socket compatibility
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef int socklen_t;

#endif

namespace Protocol {

    // TCP Message Structure
    struct TCPMessage {
        uint8_t type;           
        uint32_t dataLength;    
        std::vector<uint8_t> data;  
    };

   
    struct PositionUpdate {
        uint32_t playerId;
        float x;
        float y;
        float vx;
        float vy;
        uint64_t timestamp;
    };

    // UDP State Broadcast Structure
    struct PlayerState {
        uint32_t id;
        float x;
        float y;
        float vx;
        float vy;
    };

    struct DummyState {
        uint32_t id;
        float x;
        float y;
        uint32_t health;
        bool isAlive;
    };

    struct StateBroadcast {
        uint32_t numPlayers;
        std::vector<PlayerState> players;
        uint32_t numDummies;
        std::vector<DummyState> dummies;
    };

    // Encoding functions
    std::vector<uint8_t> encodeTCPMessage(uint8_t type, const std::vector<uint8_t>& data);
    std::vector<uint8_t> encodePositionUpdate(const PositionUpdate& update);
    std::vector<uint8_t> encodeStateBroadcast(const StateBroadcast& state);
    
    // Decoding functions
    bool decodeTCPMessage(const std::vector<uint8_t>& buffer, TCPMessage& message);
    bool decodePositionUpdate(const uint8_t* data, size_t length, PositionUpdate& update);
    bool decodeStateBroadcast(const uint8_t* data, size_t length, StateBroadcast& state);

    // Helper function for network byte order conversion
    template<typename T>
    T ntoh(T value);
    
    template<typename T>
    T hton(T value);

    // Initialize network subsystem (Windows specific)
    bool initializeNetwork();
    void cleanupNetwork();
}

#endif 


#endif
