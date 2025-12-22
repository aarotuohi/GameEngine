cmake_minimum_required(VERSION 3.15)
project(GameEngine VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Platform-specific settings
if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0601)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
    set(PLATFORM_LIBS ws2_32)
elseif(UNIX)
    set(PLATFORM_LIBS pthread)
endif()

# Find SDL2 for client
find_package(SDL2 QUIET)
find_package(SDL2_mixer QUIET)
if(SDL2_FOUND AND SDL2_mixer_FOUND)
    set(BUILD_CLIENT ON)
    message(STATUS "SDL2 and SDL2_mixer found - Building client")
elseif(SDL2_FOUND)
    set(BUILD_CLIENT ON)
    message(WARNING "SDL2_mixer not found - Building client without audio support")
else()
    set(BUILD_CLIENT OFF)
    message(WARNING "SDL2 not found - Skipping client build")
endif()

# Shared library
add_library(SharedLib STATIC
    shared/Config.h
    shared/Protocol.h
    shared/Protocol.cpp
    shared/Entities.h
    shared/Entities.cpp
)

target_include_directories(SharedLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Server executable
add_executable(GameServer
    server/main.cpp
    server/GameServer.h
    server/GameServer.cpp
    server/GameState.h
    server/GameState.cpp
    server/NetworkHandler.h
    server/NetworkHandler.cpp
    server/PlayerManager.h
    server/PlayerManager.cpp
)

target_link_libraries(GameServer PRIVATE SharedLib ${PLATFORM_LIBS})
target_include_directories(GameServer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Client executable (if SDL2 is available)
if(BUILD_CLIENT)
    add_executable(GameClient
        client/main.cpp
        client/GameClient.h
        client/GameClient.cpp
        client/NetworkManager.h
        client/NetworkManager.cpp
        client/InputHandler.h
        client/InputHandler.cpp
        client/Renderer.h
        client/Renderer.cpp
        client/AudioManager.h
        client/AudioManager.cpp
    )

    set(CLIENT_LIBS SharedLib ${SDL2_LIBRARIES} ${PLATFORM_LIBS})
    
  
    if(SDL2_mixer_FOUND)
        list(APPEND CLIENT_LIBS ${SDL2_mixer_LIBRARIES})
        target_compile_definitions(GameClient PRIVATE HAS_AUDIO_SUPPORT)
    endif()
    
    target_link_libraries(GameClient PRIVATE ${CLIENT_LIBS})
    
    set(CLIENT_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR} ${SDL2_INCLUDE_DIRS})
    if(SDL2_mixer_FOUND)
        list(APPEND CLIENT_INCLUDES ${SDL2_mixer_INCLUDE_DIRS})
    endif()
    
    target_include_directories(GameClient PRIVATE ${CLIENT_INCLUDES})
endif()

# Install targets
install(TARGETS GameServer DESTINATION bin)
if(BUILD_CLIENT)
    install(TARGETS GameClient DESTINATION bin)
endif()
