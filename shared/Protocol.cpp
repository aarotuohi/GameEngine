#include "Protocol.h"
#include <cstring>
#include <chrono>

namespace Protocol {

    // Network initialization
    bool initializeNetwork() {
        #ifdef _WIN32
        WSADATA wsaData;
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        return result == 0;
        #else
        return true;  // No initialization needed on Unix
        #endif
    }

    void cleanupNetwork() {
        #ifdef _WIN32
        WSACleanup();
        #endif
    }

    // Byte order conversion
    template<>
    uint16_t ntoh(uint16_t value) {
        return ntohs(value);
    }

    template<>
    uint32_t ntoh(uint32_t value) {
        return ntohl(value);
    }

    template<>
    uint16_t hton(uint16_t value) {
        return htons(value);
    }

    template<>
    uint32_t hton(uint32_t value) {
        return htonl(value);
    }

    // TCP Message encoding
    std::vector<uint8_t> encodeTCPMessage(uint8_t type, const std::vector<uint8_t>& data) {
        std::vector<uint8_t> buffer;
        uint32_t length = hton(static_cast<uint32_t>(data.size()));
        
        // Add length (4 bytes)
        buffer.insert(buffer.end(), 
                     reinterpret_cast<uint8_t*>(&length),
                     reinterpret_cast<uint8_t*>(&length) + sizeof(length));
        
        // Add type (1 byte)
        buffer.push_back(type);
        
        // Add data
        buffer.insert(buffer.end(), data.begin(), data.end());
        
        return buffer;
    }

    // TCP Message decoding
    bool decodeTCPMessage(const std::vector<uint8_t>& buffer, TCPMessage& message) {
        if (buffer.size() < 5) return false;  // Minimum: 4 bytes length + 1 byte type
        
        // Extract length
        uint32_t length;
        std::memcpy(&length, buffer.data(), sizeof(length));
        length = ntoh(length);
        
        if (buffer.size() < 5 + length) return false;
        
        // Extract type
        message.type = buffer[4];
        
        // Extract data
        message.dataLength = length;
        message.data.assign(buffer.begin() + 5, buffer.begin() + 5 + length);
        
        return true;
    }

    // Position update encoding
    std::vector<uint8_t> encodePositionUpdate(const PositionUpdate& update) {
        std::vector<uint8_t> buffer(sizeof(PositionUpdate));
        
        // Convert to network byte order
        PositionUpdate netUpdate = update;
        netUpdate.playerId = hton(update.playerId);
        
        // Copy to buffer
        std::memcpy(buffer.data(), &netUpdate, sizeof(PositionUpdate));
        
        return buffer;
    }

    // Position update decoding
    bool decodePositionUpdate(const uint8_t* data, size_t length, PositionUpdate& update) {
        if (length < sizeof(PositionUpdate)) return false;
        
        std::memcpy(&update, data, sizeof(PositionUpdate));
        update.playerId = ntoh(update.playerId);
        
        return true;
    }

    // State broadcast encoding
    std::vector<uint8_t> encodeStateBroadcast(const StateBroadcast& state) {
        std::vector<uint8_t> buffer;
        
        // Add number of players
        uint32_t numPlayers = hton(state.numPlayers);
        buffer.insert(buffer.end(),
                     reinterpret_cast<uint8_t*>(&numPlayers),
                     reinterpret_cast<uint8_t*>(&numPlayers) + sizeof(numPlayers));
        
        // Add each player state
        for (const auto& player : state.players) {
            PlayerState netPlayer = player;
            netPlayer.id = hton(player.id);
            
            buffer.insert(buffer.end(),
                         reinterpret_cast<const uint8_t*>(&netPlayer),
                         reinterpret_cast<const uint8_t*>(&netPlayer) + sizeof(PlayerState));
        }
        
        return buffer;
    }

    // State broadcast decoding
    bool decodeStateBroadcast(const uint8_t* data, size_t length, StateBroadcast& state) {
        if (length < sizeof(uint32_t)) return false;
        
        // Extract number of players
        std::memcpy(&state.numPlayers, data, sizeof(uint32_t));
        state.numPlayers = ntoh(state.numPlayers);
        
        size_t expectedSize = sizeof(uint32_t) + state.numPlayers * sizeof(PlayerState);
        if (length < expectedSize) return false;
        
        // Extract player states
        state.players.clear();
        const uint8_t* ptr = data + sizeof(uint32_t);
        
        for (uint32_t i = 0; i < state.numPlayers; ++i) {
            PlayerState player;
            std::memcpy(&player, ptr, sizeof(PlayerState));
            player.id = ntoh(player.id);
            state.players.push_back(player);
            ptr += sizeof(PlayerState);
        }
        
        return true;
    }
}
