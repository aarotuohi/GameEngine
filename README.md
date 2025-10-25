Multiplayer Game

2D multiplayer action game featuring characters that I like. Built with C++ and SDL2.

How to Play

Movement: Right-click anywhere to move

Abilites: Q, W, E, R

Only one character at this point

'ESC' to quit

Quick Start

Build:
```bash
mkdir build && cd build
cmake ..
cmake --build . --config Debug
```

Run:
```bash
# Start server first
cd build/bin/Debug
./GameServer.exe

# Then start client(s)
./GameClient.exe
```

Features

- Real-time multiplayer (TCP + UDP networking)
- Infinite procedurally generated world
- Q ability with stacking mechanic
- Beautiful tornado visuals
- Target dummies for practice
- Smooth 144 FPS gameplay

Tech Stack

C++17 - Modern C++ with threading
SDL2 - 2D rendering
CMake- Build system
Winsock2- Networking (TCP/UDP)
