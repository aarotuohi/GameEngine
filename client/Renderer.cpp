#include "Renderer.h"
#include <iostream>
#include <cmath>

Renderer::Renderer(int w, int h)
    : window(nullptr), renderer(nullptr), width(w), height(h),
      cameraX(0.0f), cameraY(0.0f), cameraScale(1.0f),
      bgColor{30, 30, 40, 255},           // Dark blue-gray
      grassColor1{85, 140, 60, 255},      // Base grass green
      grassColor2{70, 120, 50, 255},      // Darker grass
      grassColor3{95, 150, 70, 255},      // Lighter grass accent
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

void Renderer::updateCamera(float playerX, float playerY) {
  
    // No world bounds - infinite world generation
    cameraScale = 1.0f;
    
    // Center camera on player
    cameraX = playerX - (width / 2.0f);
    cameraY = playerY - (height / 2.0f);
}

void Renderer::worldToScreen(float worldX, float worldY, int& screenX, int& screenY) {
    screenX = static_cast<int>((worldX - cameraX) * cameraScale);
    screenY = static_cast<int>((worldY - cameraY) * cameraScale);
}

void Renderer::screenToWorld(int screenX, int screenY, float& worldX, float& worldY) {
    worldX = (static_cast<float>(screenX) / cameraScale) + cameraX;
    worldY = (static_cast<float>(screenY) / cameraScale) + cameraY;
}

void Renderer::renderGrass() {
    // Draw base grass layer
    setColor(grassColor1);
    SDL_RenderClear(renderer);
    
    // Calculate visible world area 
    float worldStartX = cameraX;
    float worldStartY = cameraY;
    float worldEndX = cameraX + width;
    float worldEndY = cameraY + height;
    
    // Draw grass patternworks for any world position
    const int grassSize = 20;
    const int grassBladeHeight = 8;
    
    for (int worldX = static_cast<int>(worldStartX / grassSize) * grassSize; 
         worldX < worldEndX; worldX += grassSize) {
        for (int worldY = static_cast<int>(worldStartY / grassSize) * grassSize; 
             worldY < worldEndY; worldY += grassSize) {
            
            // Create variation using position-based random
            int variation = (worldX * 7 + worldY * 13) % 3;
            
            int screenX, screenY;
            worldToScreen(static_cast<float>(worldX), static_cast<float>(worldY), screenX, screenY);
            int scaledSize = static_cast<int>(grassSize * cameraScale);
            int scaledBladeHeight = static_cast<int>(grassBladeHeight * cameraScale);
            
            // Draw grass patch with darker/lighter variations
            if (variation == 0) {
                setColor(grassColor2);
            } else if (variation == 1) {
                setColor(grassColor3);
            } else {
                setColor(grassColor1);
            }
            
            SDL_Rect grassPatch = {screenX, screenY, scaledSize, scaledSize};
            SDL_RenderFillRect(renderer, &grassPatch);
            
            // Draw grass blades on top for detail
            setColor(grassColor2);
            for (int i = 0; i < 3; i++) {
                int bladeX = screenX + (i * scaledSize / 3) + (scaledSize / 6);
                int bladeY = screenY + scaledSize / 2;
                SDL_Rect blade = {bladeX, bladeY, (std::max)(1, scaledSize / 10), scaledBladeHeight};
                SDL_RenderFillRect(renderer, &blade);
            }
        }
    }
}

void Renderer::renderGrid() {
    setColor(gridColor);
    const int gridSize = 50;
    
    // Calculate visible world area (
    float worldStartX = cameraX;
    float worldStartY = cameraY;
    float worldEndX = cameraX + width;
    float worldEndY = cameraY + height;
    
    // Vertical lines 
    for (int worldX = static_cast<int>(worldStartX / gridSize) * gridSize; 
         worldX < worldEndX; worldX += gridSize) {
        int screenX, screenY1, screenY2;
        worldToScreen(static_cast<float>(worldX), worldStartY, screenX, screenY1);
        worldToScreen(static_cast<float>(worldX), worldEndY, screenX, screenY2);
        SDL_RenderDrawLine(renderer, screenX, 0, screenX, height);
    }
    
    // Horizontal lines 
    for (int worldY = static_cast<int>(worldStartY / gridSize) * gridSize; 
         worldY < worldEndY; worldY += gridSize) {
        int screenX1, screenY, screenX2;
        worldToScreen(worldStartX, static_cast<float>(worldY), screenX1, screenY);
        worldToScreen(worldEndX, static_cast<float>(worldY), screenX2, screenY);
        SDL_RenderDrawLine(renderer, 0, screenY, width, screenY);
    }
}

bool Renderer::shouldSpawnDecoration(int worldX, int worldY, int decorationType) {

    // spawn rates and distribution based on random hash
    int hash = (worldX * 73 + worldY * 149 + decorationType * 97) % 1000;
    
    // All decorations have the same spawn rate: 5%
    // Cabins: 5%
    if (decorationType == 0) return hash < 50;
    
    // Spruces: 5%
    if (decorationType == 1) return hash < 50;

    // Campfires: 5%
    if (decorationType == 2) return hash < 50;
    
    return false;
}

void Renderer::renderCabin(int worldX, int worldY) {
    int screenX, screenY;
    worldToScreen(static_cast<float>(worldX), static_cast<float>(worldY), screenX, screenY);
    
    int scale = static_cast<int>(cameraScale);
    
    // Cabin base
    SDL_Color cabinWood = {139, 90, 60, 255};
    SDL_Color cabinRoof = {100, 50, 30, 255};
    SDL_Color cabinWindow = {255, 220, 150, 255}; 
    SDL_Color cabinDoor = {80, 50, 30, 255};
    
    // Main cabin structure
    setColor(cabinWood);
    SDL_Rect cabin = {screenX, screenY, 40 * scale, 30 * scale};
    SDL_RenderFillRect(renderer, &cabin);
    
    // Roof 
    setColor(cabinRoof);
    int roofHeight = 15 * scale;
    int roofWidth = 50 * scale; 
    
    // Draw triangle roof
    for (int i = 0; i < roofHeight; i++) {
        
        int currentWidth = (roofWidth * i) / roofHeight;
        int xOffset = (roofWidth - currentWidth) / 2;
        
        SDL_Rect roofLine = {
            screenX - 5 * scale + xOffset, 
            screenY - roofHeight + i, 
            currentWidth, 
            1
        };
        SDL_RenderFillRect(renderer, &roofLine);
    }
    
    // Windows
    setColor(cabinWindow);
    SDL_Rect window1 = {screenX + 5 * scale, screenY + 8 * scale, 8 * scale, 8 * scale};
    SDL_Rect window2 = {screenX + 27 * scale, screenY + 8 * scale, 8 * scale, 8 * scale};
    SDL_RenderFillRect(renderer, &window1);
    SDL_RenderFillRect(renderer, &window2);
    
    // Door
    setColor(cabinDoor);
    SDL_Rect door = {screenX + 16 * scale, screenY + 15 * scale, 8 * scale, 15 * scale};
    SDL_RenderFillRect(renderer, &door);
    
    // Chimney
    setColor(cabinRoof);
    SDL_Rect chimney = {screenX + 30 * scale, screenY - 20 * scale, 6 * scale, 10 * scale};
    SDL_RenderFillRect(renderer, &chimney);
    
    // Smoke from chimney
    SDL_Color smoke = {200, 200, 200, 150};
    setColor(smoke);
    for (int i = 0; i < 3; i++) {
        SDL_Rect smokeParticle = {
            screenX + 32 * scale + (i * 2), 
            screenY - 25 * scale - (i * 5), 
            3 * scale, 3 * scale
        };
        SDL_RenderFillRect(renderer, &smokeParticle);
    }
}

void Renderer::renderSpruce(int worldX, int worldY) {
    int screenX, screenY;
    worldToScreen(static_cast<float>(worldX), static_cast<float>(worldY), screenX, screenY);
    
    int scale = static_cast<int>(cameraScale);
    
    // Tree colors
    SDL_Color trunk = {101, 67, 33, 255};
    SDL_Color needles1 = {34, 80, 49, 255}; // Dark 
    SDL_Color needles2 = {45, 95, 60, 255}; // Medium 
    
    // Trunk 
    setColor(trunk);
    SDL_Rect trunkRect = {screenX + 13 * scale, screenY - 5 * scale, 4 * scale, 40 * scale};
    SDL_RenderFillRect(renderer, &trunkRect);
    
    
    // Bottom layer 
    setColor(needles1);
    for (int i = 0; i < 15 * scale; i++) {
        int layerWidth = 30 * scale - i * 2;
        SDL_Rect layer = {screenX + i, screenY + 20 * scale - i, layerWidth, 1};
        SDL_RenderFillRect(renderer, &layer);
    }
    
    // Middle layer
    setColor(needles2);
    for (int i = 0; i < 12 * scale; i++) {
        int layerWidth = 24 * scale - i * 2;
        SDL_Rect layer = {screenX + 3 * scale + i, screenY + 8 * scale - i, layerWidth, 1};
        SDL_RenderFillRect(renderer, &layer);
    }
    
    // Top layer 
    setColor(needles1);
    for (int i = 0; i < 8 * scale; i++) {
        int layerWidth = 16 * scale - i * 2;
        SDL_Rect layer = {screenX + 7 * scale + i, screenY - 5 * scale - i, layerWidth, 1};
        SDL_RenderFillRect(renderer, &layer);
    }
}

void Renderer::renderCampfire(int worldX, int worldY) {
    int screenX, screenY;
    worldToScreen(static_cast<float>(worldX), static_cast<float>(worldY), screenX, screenY);
    
    int scale = static_cast<int>(cameraScale);
    
    // Fire colors
    SDL_Color logs = {101, 67, 33, 255};
    SDL_Color fireYellow = {255, 200, 0, 255};
    SDL_Color fireOrange = {255, 100, 0, 255};
    SDL_Color fireRed = {200, 0, 0, 255};
    
    // Logs arranged in a circle
    setColor(logs);
    SDL_Rect log1 = {screenX + 5 * scale, screenY + 10 * scale, 15 * scale, 3 * scale};
    SDL_Rect log2 = {screenX + 10 * scale, screenY + 5 * scale, 3 * scale, 15 * scale};
    SDL_RenderFillRect(renderer, &log1);
    SDL_RenderFillRect(renderer, &log2);

    // layers of fire
    setColor(fireRed);
    for (int i = 0; i < 8 * scale; i++) {
        int flameWidth = 8 * scale - i;
        SDL_Rect flame = {screenX + 8 * scale + i / 2, screenY + 8 * scale - i, flameWidth, 1};
        SDL_RenderFillRect(renderer, &flame);
    }
    
    setColor(fireOrange);
    for (int i = 0; i < 6 * scale; i++) {
        int flameWidth = 6 * scale - i;
        SDL_Rect flame = {screenX + 9 * scale + i / 2, screenY + 6 * scale - i, flameWidth, 1};
        SDL_RenderFillRect(renderer, &flame);
    }
    
    setColor(fireYellow);
    for (int i = 0; i < 4 * scale; i++) {
        int flameWidth = 4 * scale - i;
        SDL_Rect flame = {screenX + 10 * scale + i / 2, screenY + 4 * scale - i, flameWidth, 1};
        SDL_RenderFillRect(renderer, &flame);
    }
    
    // Sparks above fire
    SDL_Color sparks = {255, 150, 0, 200};
    setColor(sparks);
    SDL_Rect spark1 = {screenX + 9 * scale, screenY - 3 * scale, 1, 1};
    SDL_Rect spark2 = {screenX + 13 * scale, screenY - 5 * scale, 1, 1};
    SDL_RenderFillRect(renderer, &spark1);
    SDL_RenderFillRect(renderer, &spark2);
}

void Renderer::renderDecorations() {

    // wrolds area for decorations
    float worldStartX = cameraX - 100;
    float worldStartY = cameraY - 100;
    float worldEndX = cameraX + width + 100;
    float worldEndY = cameraY + height + 100;
    
    // Grid size for decoration placement 
    const int decorationGrid = 100;
    
    // grid and place decorations
    for (int worldX = static_cast<int>(worldStartX / decorationGrid) * decorationGrid; 
         worldX < worldEndX; worldX += decorationGrid) {
        for (int worldY = static_cast<int>(worldStartY / decorationGrid) * decorationGrid; 
             worldY < worldEndY; worldY += decorationGrid) {
            
            // Cabins (highest priority)
            if (shouldSpawnDecoration(worldX, worldY, 0)) {
                renderCabin(worldX + 30, worldY + 30);
                continue; 
            }
            
            // Spruces (medium priority)
            if (shouldSpawnDecoration(worldX, worldY, 1)) {
                renderSpruce(worldX + 10, worldY + 10);
                continue; 
            }
            
            // Campfires (lowest priority)
            if (shouldSpawnDecoration(worldX, worldY, 2)) {
                renderCampfire(worldX + 20, worldY + 20);
            }
        }
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
    
    // Convert world coordinates to screen coordinates
    int centerX, centerY;
    worldToScreen(player.x, player.y, centerX, centerY);
    
    
    int scale = static_cast<int>(2 * cameraScale); 

    
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
    
    // Draw legs 
    setColor(armorDark);
    SDL_Rect leftLeg = {centerX - 4 * scale, centerY + 6 * scale, 3 * scale, 8 * scale};
    SDL_Rect rightLeg = {centerX + 1 * scale, centerY + 6 * scale, 3 * scale, 8 * scale};
    SDL_RenderFillRect(renderer, &leftLeg);
    SDL_RenderFillRect(renderer, &rightLeg);
    
    // Draw body 
    setColor(armorBlue);
    SDL_Rect body = {centerX - 5 * scale, centerY - 2 * scale, 10 * scale, 10 * scale};
    SDL_RenderFillRect(renderer, &body);
    
    // Draw armor details 
    setColor(armorDark);
    SDL_Rect armorDetail1 = {centerX - 3 * scale, centerY, 2 * scale, 6 * scale};
    SDL_Rect armorDetail2 = {centerX + 1 * scale, centerY, 2 * scale, 6 * scale};
    SDL_RenderFillRect(renderer, &armorDetail1);
    SDL_RenderFillRect(renderer, &armorDetail2);
    
    // Draw head 
    setColor(skinColor);
    SDL_Rect head = {centerX - 3 * scale, centerY - 8 * scale, 6 * scale, 6 * scale};
    SDL_RenderFillRect(renderer, &head);
    
    // Draw hair 
    setColor(hairColor);
    SDL_Rect hair1 = {centerX - 4 * scale, centerY - 10 * scale, 8 * scale, 4 * scale};
    SDL_RenderFillRect(renderer, &hair1);

    // Hair ponytail flowing
    SDL_Rect ponytail = {centerX - 8 * scale, centerY - 8 * scale, 5 * scale, 3 * scale};
    SDL_RenderFillRect(renderer, &ponytail);
    
    // Draw eyes 
    setColor({255, 255, 255, 255});
    SDL_Rect leftEye = {centerX - 2 * scale, centerY - 6 * scale, 1 * scale, 1 * scale};
    SDL_Rect rightEye = {centerX + 1 * scale, centerY - 6 * scale, 1 * scale, 1 * scale};
    SDL_RenderFillRect(renderer, &leftEye);
    SDL_RenderFillRect(renderer, &rightEye);
    
    // Draw sword
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

    // wind effect WIP
    if (player.isDashing || player.activeAbility != SamuraiAbility::NONE) {
        setColor(windColor);
        // Circular wind particles
        for (int i = 0; i < 360; i += 40) {
            float angle = i * 3.14159f / 180.0f;
            int windRadius = static_cast<int>(18 * cameraScale);
            int wx = centerX + static_cast<int>(std::cos(angle) * windRadius);
            int wy = centerY + static_cast<int>(std::sin(angle) * windRadius);
            SDL_Rect windParticle = {wx, wy, (std::max)(1, static_cast<int>(2 * cameraScale)), 
                                     (std::max)(1, static_cast<int>(4 * cameraScale))};
            SDL_RenderFillRect(renderer, &windParticle);
        }
        
        // Add flowing wind lines
        for (int i = 0; i < 3; i++) {
            int lineY = centerY - static_cast<int>(10 * cameraScale) + static_cast<int>(i * 8 * cameraScale);
            SDL_RenderDrawLine(renderer, 
                centerX - static_cast<int>(20 * cameraScale), lineY, 
                centerX - static_cast<int>(10 * cameraScale), lineY);
            SDL_RenderDrawLine(renderer, 
                centerX + static_cast<int>(10 * cameraScale), lineY, 
                centerX + static_cast<int>(20 * cameraScale), lineY);
        }
    }
    
    // Health bar above character
    int barWidth = static_cast<int>(40 * cameraScale);
    int barHeight = static_cast<int>(5 * cameraScale);
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
    renderText(player.name.c_str(), centerX - static_cast<int>(20 * cameraScale), 
               centerY + static_cast<int>(20 * cameraScale), static_cast<int>(12 * cameraScale));
    
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
