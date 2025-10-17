# Distributed Multiplayer Game Engine (C++)

A simple 2D multiplayer game engine with real-time synchronization and distributed architecture built in modern C++.

## Features

- **Server-side game state management**: Central authority for game logic
- **Real-time position updates**: UDP for fast, low-latency updates
- **Reliable messaging**: TCP for critical game events
- **Concurrent client handling**: Multi-threaded server architecture with std::thread
- **Efficient synchronization**: Binary protocol with delta compression
- **Cross-platform**: Windows, Linux, macOS support

## Architecture

```
Client 1 ←→ TCP/UDP ←→ Server ←→ TCP/UDP ←→ Client 2
                          ↓
                     Game State
```

## Getting Started

### Prerequisites

**Windows:**
- Visual Studio 2019+ with C++ Desktop Development
- CMake 3.15+
- SDL2 library

**Linux:**
```bash
sudo apt-get install build-essential cmake libsdl2-dev libsdl2-ttf-dev
```

**macOS:**
```bash
brew install cmake sdl2 sdl2_ttf
```

### Building

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

### Running the Server

```bash
./bin/GameServer
```

### Running a Client

```bash
./bin/GameClient
```

## Project Structure

- `server/` - Server-side code
  - `main.cpp` - Main server entry point
  - `GameServer.h/cpp` - Server implementation
  - `GameState.h/cpp` - Game state management
  - `NetworkHandler.h/cpp` - TCP/UDP communication
  - `PlayerManager.h/cpp` - Player connection management

- `client/` - Client-side code
  - `main.cpp` - Main client entry point
  - `GameClient.h/cpp` - Client implementation
  - `NetworkManager.h/cpp` - Network communication
  - `InputHandler.h/cpp` - Player input processing
  - `Renderer.h/cpp` - SDL2 rendering

- `shared/` - Shared code between server and client
  - `Protocol.h/cpp` - Network protocol definitions
  - `Entities.h/cpp` - Game entity classes
  - `Config.h` - Configuration constants

## Game Concepts

This is a simple 2D tag/shooter game where:
- Players move in a shared 2D world
- Real-time position synchronization
- Simple collision detection
- Tagging/shooting mechanics

## Extensions

- UDP for position updates (low latency)
- TCP for reliable game events (player join/leave, scoring)
- Client-side prediction
- Server reconciliation
- Lag compensation
