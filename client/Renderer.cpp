#include "Renderer.h"
#include <iostream>
#include <cmath>

Renderer::Renderer(int w, int h)
    : window(nullptr), renderer(nullptr), width(w), height(h),
      bgColor{50, 50, 50, 255},
      playerColor{100, 200, 255, 255},
      taggedColor{255, 100, 100, 255},
      otherPlayerColor{150, 150, 150, 255},
      gridColor{70, 70, 70, 255},
      textColor{255, 255, 255, 255} {
}

Renderer::~Renderer() {
    if (renderer) {
        SDL_DestroyRenderer(renderer);
    }
    if (window) {
        SDL_DestroyWindow(window);
    }
    SDL_Quit();
}

bool Renderer::initialize() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL initialization failed: " << SDL_GetError() << "\n";
        return false;
    }
    
    window = SDL_CreateWindow("Multiplayer Game Engine",
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              width, height, SDL_WINDOW_SHOWN);
    
    if (!window) {
        std::cerr << "Window creation failed: " << SDL_GetError() << "\n";
        return false;
    }
    
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    
    if (!renderer) {
        std::cerr << "Renderer creation failed: " << SDL_GetError() << "\n";
        return false;
    }
    
    return true;
}

void Renderer::clear() {
    setColor(bgColor);
    SDL_RenderClear(renderer);
}

void Renderer::present() {
    SDL_RenderPresent(renderer);
}

void Renderer::setColor(const SDL_Color& color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
}

void Renderer::renderGrid() {
    setColor(gridColor);
    const int gridSize = 50;
    
    // Vertical lines
    for (int x = 0; x < width; x += gridSize) {
        SDL_RenderDrawLine(renderer, x, 0, x, height);
    }
    
    // Horizontal lines
    for (int y = 0; y < height; y += gridSize) {
        SDL_RenderDrawLine(renderer, 0, y, width, y);
    }
}

void Renderer::renderCircle(int centerX, int centerY, int radius) {
    const int diameter = radius * 2;
    
    for (int w = 0; w < diameter; w++) {
        for (int h = 0; h < diameter; h++) {
            int dx = radius - w;
            int dy = radius - h;
            if ((dx * dx + dy * dy) <= (radius * radius)) {
                SDL_RenderDrawPoint(renderer, centerX + dx, centerY + dy);
            }
        }
    }
}

void Renderer::renderPlayer(const Player& player, bool isLocal) {
    // Choose color
    SDL_Color color;
    if (player.isTagged) {
        color = taggedColor;
    } else if (isLocal) {
        color = playerColor;
    } else {
        color = otherPlayerColor;
    }
    
    // Draw player circle
    setColor(color);
    renderCircle(static_cast<int>(player.x), static_cast<int>(player.y), 
                static_cast<int>(player.size / 2));
    
    // Draw outline
    setColor({0, 0, 0, 255});
    for (int i = 0; i < 360; i += 10) {
        float angle = i * 3.14159f / 180.0f;
        int x1 = static_cast<int>(player.x + std::cos(angle) * player.size / 2);
        int y1 = static_cast<int>(player.y + std::sin(angle) * player.size / 2);
        int x2 = static_cast<int>(player.x + std::cos(angle + 0.1f) * player.size / 2);
        int y2 = static_cast<int>(player.y + std::sin(angle + 0.1f) * player.size / 2);
        SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
    }
}

void Renderer::renderText(const char* text, int x, int y, int size) {
    // Simple text rendering using SDL_RenderDrawPoint
    // Note: In a full implementation, you'd use SDL_ttf for proper text rendering
    // This is a placeholder that just draws a box where text would be
    setColor(textColor);
    int textWidth = static_cast<int>(std::strlen(text)) * size / 2;
    SDL_Rect rect = {x, y, textWidth, size};
    SDL_RenderDrawRect(renderer, &rect);
}

void Renderer::renderUI(uint32_t playerId, int playerCount, int fps) {
    setColor(textColor);
    
    // Player ID indicator (top-left corner)
    SDL_Rect idRect = {10, 10, 150, 25};
    SDL_RenderDrawRect(renderer, &idRect);
    
    // Player count
    SDL_Rect countRect = {10, 40, 150, 25};
    SDL_RenderDrawRect(renderer, &countRect);
    
    // FPS counter (top-right corner)
    SDL_Rect fpsRect = {width - 80, 10, 70, 25};
    SDL_RenderDrawRect(renderer, &fpsRect);
    
    // Controls info (bottom-left)
    const char* controls[] = {
        "WASD/Arrows: Move",
        "Mouse: Aim",
        "ESC: Quit"
    };
    
    int yOffset = height - 70;
    for (const char* control : controls) {
        SDL_Rect controlRect = {10, yOffset, 200, 20};
        SDL_RenderDrawRect(renderer, &controlRect);
        yOffset += 22;
    }
}
