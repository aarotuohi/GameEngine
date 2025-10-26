#ifndef ED1984D6_E59E_4FD3_BBF6_BD65F372CBD2
#define ED1984D6_E59E_4FD3_BBF6_BD65F372CBD2
#ifndef CONFIG_H
#define CONFIG_H

#include <cstdint>
#include <string>

namespace Config {
    // Server configuration
    constexpr const char* SERVER_HOST = "127.0.0.1";
    constexpr uint16_t TCP_PORT = 5555;
    constexpr uint16_t UDP_PORT = 5556;

    // Game world settings
    constexpr int WORLD_WIDTH = 800;
    constexpr int WORLD_HEIGHT = 600;

    // Player settings
    constexpr int PLAYER_SIZE = 20;
    constexpr float PLAYER_SPEED = 200.0f;  
    constexpr int MAX_PLAYERS = 10;

    // Network settings
    constexpr int TICK_RATE = 60;         
    constexpr int UPDATE_RATE = 60;        
    constexpr int BUFFER_SIZE = 1024;

   
    constexpr float Q_SWORD_ARC_DEGREES = 90.0f; 
    constexpr float Q_SWORD_RANGE = 80.0f;       
    constexpr int Q_SWORD_DAMAGE = 20;           

    
    constexpr float E_DASH_RADIUS = 45.0f;      
    constexpr int E_DASH_DAMAGE = 30;        
    constexpr int E_DASH_VFX_MS = 200;           
    constexpr float E_DASH_DISTANCE = 200.0f;    
    constexpr int E_DASH_DURATION_MS = 180;     
    
    enum class MessageType : uint8_t {
        PLAYER_JOIN = 1,
        PLAYER_LEAVE = 2,
        GAME_STATE = 3,
        PLAYER_TAG = 4,
        PLAYER_SHOOT = 5
    };


    enum class UpdateType : uint8_t {
        POSITION = 1,
        VELOCITY = 2,
        STATE_BROADCAST = 3,
        ABILITY_USE = 4
    };
}

#endif 


#endif 
