#include "Renderer.h"
#include <iostream>
#include <cmath>

Renderer::Renderer(int w, int h)
    : window(nullptr), renderer(nullptr), width(w), height(h),
      bgColor{30, 30, 40, 255},           // Dark blue-gray
      samuraiColor{0, 180, 255, 255},       //  teal/cyan
      samuraiSwordColor{220, 220, 255, 255}, // Silver-white
      windColor{100, 200, 255, 180},      // Light wind effect
      otherPlayerColor{200, 50, 50, 255}, // Enemy red
      gridColor{50, 50, 60, 255},
      textColor{255, 255, 255, 255},
      healthBarGreen{50, 255, 50, 255},
      healthBarRed{255, 50, 50, 255} {
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
    if (!player.isAlive) return; 
    
    int centerX = static_cast<int>(player.x);
    int centerY = static_cast<int>(player.y);
    
    // Samurai pixel art style rendering
    int scale = 2; // Scale factor for pixel art

    // Define Samurai's colors
    SDL_Color hairColor = {60, 50, 80, 255};        // Dark purple hair
    SDL_Color skinColor = {255, 220, 190, 255};     // Skin tone
    SDL_Color armorBlue = {80, 150, 200, 255};      // Blue armor
    SDL_Color armorDark = {50, 90, 130, 255};       // Dark blue
    SDL_Color capeColor = {100, 120, 180, 255};     // Purple-blue cape
    SDL_Color swordGray = {180, 180, 190, 255};     // Sword metal
    SDL_Color swordHandle = {100, 70, 50, 255};     // Sword handle
    SDL_Color enemyColor = {200, 50, 50, 255};      // Enemy red
    
    if (!isLocal) {
        // Make enemies red-tinted
        armorBlue = enemyColor;
        armorDark = {150, 30, 30, 255};
    }

    // Draw flowing cape behind Samurai
    setColor(capeColor);
    // Cape flows to the left
    for (int i = 0; i < 8; i++) {
        int capeX = centerX - 8 - i;
        int capeY = centerY - 10 + (i % 3);
        SDL_Rect capeRect = {capeX, capeY, 3 * scale, 12 * scale};
        SDL_RenderFillRect(renderer, &capeRect);
    }
    
    // Draw legs (dark blue pants)
    setColor(armorDark);
    SDL_Rect leftLeg = {centerX - 4 * scale, centerY + 6 * scale, 3 * scale, 8 * scale};
    SDL_Rect rightLeg = {centerX + 1 * scale, centerY + 6 * scale, 3 * scale, 8 * scale};
    SDL_RenderFillRect(renderer, &leftLeg);
    SDL_RenderFillRect(renderer, &rightLeg);
    
    // Draw body (blue armor)
    setColor(armorBlue);
    SDL_Rect body = {centerX - 5 * scale, centerY - 2 * scale, 10 * scale, 10 * scale};
    SDL_RenderFillRect(renderer, &body);
    
    // Draw armor details (darker blue accents)
    setColor(armorDark);
    SDL_Rect armorDetail1 = {centerX - 3 * scale, centerY, 2 * scale, 6 * scale};
    SDL_Rect armorDetail2 = {centerX + 1 * scale, centerY, 2 * scale, 6 * scale};
    SDL_RenderFillRect(renderer, &armorDetail1);
    SDL_RenderFillRect(renderer, &armorDetail2);
    
    // Draw head (skin)
    setColor(skinColor);
    SDL_Rect head = {centerX - 3 * scale, centerY - 8 * scale, 6 * scale, 6 * scale};
    SDL_RenderFillRect(renderer, &head);
    
    // Draw hair (purple/black flowing)
    setColor(hairColor);
    SDL_Rect hair1 = {centerX - 4 * scale, centerY - 10 * scale, 8 * scale, 4 * scale};
    SDL_RenderFillRect(renderer, &hair1);
    // Hair ponytail flowing
    SDL_Rect ponytail = {centerX - 8 * scale, centerY - 8 * scale, 5 * scale, 3 * scale};
    SDL_RenderFillRect(renderer, &ponytail);
    
    // Draw eyes (small white dots)
    setColor({255, 255, 255, 255});
    SDL_Rect leftEye = {centerX - 2 * scale, centerY - 6 * scale, 1 * scale, 1 * scale};
    SDL_Rect rightEye = {centerX + 1 * scale, centerY - 6 * scale, 1 * scale, 1 * scale};
    SDL_RenderFillRect(renderer, &leftEye);
    SDL_RenderFillRect(renderer, &rightEye);
    
    // Draw sword (katana extending to the right)
    setColor(swordHandle);
    SDL_Rect swordHandle_rect = {centerX + 2 * scale, centerY - 2 * scale, 3 * scale, 8 * scale};
    SDL_RenderFillRect(renderer, &swordHandle_rect);
    
    setColor(swordGray);
    // Sword blade
    SDL_Rect blade = {centerX + 4 * scale, centerY - 8 * scale, 2 * scale, 20 * scale};
    SDL_RenderFillRect(renderer, &blade);
    // Sword tip
    SDL_Rect tip = {centerX + 5 * scale, centerY + 11 * scale, 1 * scale, 3 * scale};
    SDL_RenderFillRect(renderer, &tip);
    
    // Sword shine effect
    setColor({255, 255, 255, 200});
    SDL_Rect shine = {centerX + 4 * scale, centerY, 1 * scale, 6 * scale};
    SDL_RenderFillRect(renderer, &shine);

    // Wind effect around Samurai when dashing or using abilities
    if (player.isDashing || player.activeAbility != SamuraiAbility::NONE) {
        setColor(windColor);
        // Circular wind particles
        for (int i = 0; i < 360; i += 40) {
            float angle = i * 3.14159f / 180.0f;
            int windRadius = 18;
            int wx = centerX + static_cast<int>(std::cos(angle) * windRadius);
            int wy = centerY + static_cast<int>(std::sin(angle) * windRadius);
            SDL_Rect windParticle = {wx, wy, 2, 4};
            SDL_RenderFillRect(renderer, &windParticle);
        }
        
        // Add flowing wind lines
        for (int i = 0; i < 3; i++) {
            int lineY = centerY - 10 + i * 8;
            SDL_RenderDrawLine(renderer, centerX - 20, lineY, centerX - 10, lineY);
            SDL_RenderDrawLine(renderer, centerX + 10, lineY, centerX + 20, lineY);
        }
    }
    
    // Health bar above character
    int barWidth = 40;
    int barHeight = 5;
    int barX = centerX - barWidth / 2;
    int barY = centerY - 25 * scale;
    
    // Background (red)
    SDL_Rect bgRect = {barX, barY, barWidth, barHeight};
    setColor(healthBarRed);
    SDL_RenderFillRect(renderer, &bgRect);
    
    // Foreground (green based on health percentage)
    float healthPercent = static_cast<float>(player.health) / player.maxHealth;
    SDL_Rect fgRect = {barX, barY, static_cast<int>(barWidth * healthPercent), barHeight};
    setColor(healthBarGreen);
    SDL_RenderFillRect(renderer, &fgRect);
    
    // Player name
    setColor(textColor);
    renderText(player.name.c_str(), centerX - 20, centerY + 20, 12);
    
    // Draw ability indicators for local player
    if (isLocal) {
        // Q stacks indicator
        for (int i = 0; i < player.qStacks; i++) {
            SDL_Rect stackRect = {centerX - 15 + i * 12, centerY + 25, 10, 3};
            setColor(windColor);
            SDL_RenderFillRect(renderer, &stackRect);
        }
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
