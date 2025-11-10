#include "Renderer.h"
#include <iostream>
#include <cmath>

Renderer::Renderer(int w, int h)
    : window(nullptr), renderer(nullptr), width(w), height(h),
      cameraX(0.0f), cameraY(0.0f), cameraScale(1.0f),
      bgColor{30, 30, 40, 255},          
      grassColor1{85, 140, 60, 255},     
      grassColor2{70, 120, 50, 255},    
      grassColor3{95, 150, 70, 255},     
      samuraiColor{0, 180, 255, 255},      
      samuraiSwordColor{220, 220, 255, 255}, 
      windColor{100, 200, 255, 180},      
      otherPlayerColor{200, 50, 50, 255},
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
  
    cameraScale = 1.0f;
    
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

    setColor(grassColor1);
    SDL_RenderClear(renderer);

    float worldStartX = cameraX;
    float worldStartY = cameraY;
    float worldEndX = cameraX + width;
    float worldEndY = cameraY + height;

    const int grassSize = 20;
    const int grassBladeHeight = 8;
    
    for (int worldX = static_cast<int>(worldStartX / grassSize) * grassSize; 
         worldX < worldEndX; worldX += grassSize) {
        for (int worldY = static_cast<int>(worldStartY / grassSize) * grassSize; 
             worldY < worldEndY; worldY += grassSize) {
            
        
            int variation = (worldX * 7 + worldY * 13) % 3;
            
            int screenX, screenY;
            worldToScreen(static_cast<float>(worldX), static_cast<float>(worldY), screenX, screenY);
            int scaledSize = static_cast<int>(grassSize * cameraScale);
            int scaledBladeHeight = static_cast<int>(grassBladeHeight * cameraScale);
            
      
            if (variation == 0) {
                setColor(grassColor2);
            } else if (variation == 1) {
                setColor(grassColor3);
            } else {
                setColor(grassColor1);
            }
            
            SDL_Rect grassPatch = {screenX, screenY, scaledSize, scaledSize};
            SDL_RenderFillRect(renderer, &grassPatch);
            
         
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

    int seed1 = 73, seed2 = 149;
    
    if (decorationType == 0) {
    
        seed1 = 73;
        seed2 = 149;
    } else if (decorationType == 1) {
    
        seed1 = 211;
        seed2 = 317;
    } else if (decorationType == 2) {
      
        seed1 = 431;
        seed2 = 523;
    }
    
    int hash = (worldX * seed1 + worldY * seed2) % 1000;
    
  
    return hash < 50;
}

void Renderer::renderCabin(int worldX, int worldY) {
    int screenX, screenY;
    worldToScreen(static_cast<float>(worldX), static_cast<float>(worldY), screenX, screenY);
    
    int scale = static_cast<int>(cameraScale);
    

    SDL_Color cabinWood = {139, 90, 60, 255};
    SDL_Color cabinRoof = {100, 50, 30, 255};
    SDL_Color cabinWindow = {255, 220, 150, 255}; 
    SDL_Color cabinDoor = {80, 50, 30, 255};
    

    setColor(cabinWood);
    SDL_Rect cabin = {screenX, screenY, 40 * scale, 30 * scale};
    SDL_RenderFillRect(renderer, &cabin);
    

    setColor(cabinRoof);
    int roofHeight = 15 * scale;
    int roofWidth = 50 * scale; 
    
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
    
    setColor(cabinWindow);
    SDL_Rect window1 = {screenX + 5 * scale, screenY + 8 * scale, 8 * scale, 8 * scale};
    SDL_Rect window2 = {screenX + 27 * scale, screenY + 8 * scale, 8 * scale, 8 * scale};
    SDL_RenderFillRect(renderer, &window1);
    SDL_RenderFillRect(renderer, &window2);
    

    setColor(cabinDoor);
    SDL_Rect door = {screenX + 16 * scale, screenY + 15 * scale, 8 * scale, 15 * scale};
    SDL_RenderFillRect(renderer, &door);
    
    setColor(cabinRoof);
    SDL_Rect chimney = {screenX + 30 * scale, screenY - 20 * scale, 6 * scale, 10 * scale};
    SDL_RenderFillRect(renderer, &chimney);
    
  
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
    
    SDL_Color trunk = {101, 67, 33, 255};
    SDL_Color needles1 = {34, 80, 49, 255}; 
    SDL_Color needles2 = {45, 95, 60, 255}; 
    
 
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

    float worldStartX = cameraX - 100;
    float worldStartY = cameraY - 100;
    float worldEndX = cameraX + width + 100;
    float worldEndY = cameraY + height + 100;
    
    const int decorationGrid = 100;
    
    for (int worldX = static_cast<int>(worldStartX / decorationGrid) * decorationGrid; 
         worldX < worldEndX; worldX += decorationGrid) {
        for (int worldY = static_cast<int>(worldStartY / decorationGrid) * decorationGrid; 
             worldY < worldEndY; worldY += decorationGrid) {
            
          
            bool hasDecoration = false;
            
            if (!hasDecoration && shouldSpawnDecoration(worldX, worldY, 0)) {
                renderCabin(worldX + 30, worldY + 30);
                hasDecoration = true;
            }
            
            if (!hasDecoration && shouldSpawnDecoration(worldX, worldY, 1)) {
                renderSpruce(worldX + 10, worldY + 10);
                hasDecoration = true;
            }
           
            if (!hasDecoration && shouldSpawnDecoration(worldX, worldY, 2)) {
                renderCampfire(worldX + 20, worldY + 20);
            }
        }
    }
}

void Renderer::renderCircle(int centerX, int centerY, int radius) {
    
    if (radius <= 0 || radius > 500) return;
    
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
    
    int centerX, centerY;
    worldToScreen(player.x, player.y, centerX, centerY);
    
    
    int scale = static_cast<int>(2 * cameraScale); 

    bool facingRight = (player.rotation >= -1.5708f && player.rotation <= 1.5708f); 
    int flipMultiplier = facingRight ? 1 : -1;
    
    SDL_Color hairColor = {60, 50, 80, 255};        
    SDL_Color skinColor = {255, 220, 190, 255};    
    SDL_Color armorBlue = {80, 150, 200, 255};      
    SDL_Color armorDark = {50, 90, 130, 255};      
    SDL_Color capeColor = {100, 120, 180, 255};     
    SDL_Color swordGray = {180, 180, 190, 255};   
    SDL_Color swordHandle = {100, 70, 50, 255};     
    SDL_Color enemyColor = {200, 50, 50, 255};     
    
    if (!isLocal) {
    
        armorBlue = enemyColor;
        armorDark = {150, 30, 30, 255};
    }


    setColor(capeColor);
    for (int i = 0; i < 8; i++) {
        int capeX = centerX - flipMultiplier * (8 + i);
        int capeY = centerY - 10 + (i % 3);
        SDL_Rect capeRect = {capeX, capeY, 3 * scale, 12 * scale};
        SDL_RenderFillRect(renderer, &capeRect);
    }
    

    setColor(armorDark);
    SDL_Rect leftLeg = {centerX - 4 * scale, centerY + 6 * scale, 3 * scale, 8 * scale};
    SDL_Rect rightLeg = {centerX + 1 * scale, centerY + 6 * scale, 3 * scale, 8 * scale};
    SDL_RenderFillRect(renderer, &leftLeg);
    SDL_RenderFillRect(renderer, &rightLeg);

    setColor(armorBlue);
    SDL_Rect body = {centerX - 5 * scale, centerY - 2 * scale, 10 * scale, 10 * scale};
    SDL_RenderFillRect(renderer, &body);
    
 
    setColor(armorDark);
    SDL_Rect armorDetail1 = {centerX - 3 * scale, centerY, 2 * scale, 6 * scale};
    SDL_Rect armorDetail2 = {centerX + 1 * scale, centerY, 2 * scale, 6 * scale};
    SDL_RenderFillRect(renderer, &armorDetail1);
    SDL_RenderFillRect(renderer, &armorDetail2);
    
 
    setColor(skinColor);
    SDL_Rect head = {centerX - 3 * scale, centerY - 8 * scale, 6 * scale, 6 * scale};
    SDL_RenderFillRect(renderer, &head);
    
  
    setColor(hairColor);
    SDL_Rect hair1 = {centerX - 4 * scale, centerY - 10 * scale, 8 * scale, 4 * scale};
    SDL_RenderFillRect(renderer, &hair1);

    SDL_Rect ponytail = {centerX - flipMultiplier * 8 * scale, centerY - 8 * scale, 5 * scale, 3 * scale};
    SDL_RenderFillRect(renderer, &ponytail);
    
    
    setColor({255, 255, 255, 255});
    SDL_Rect leftEye = {centerX - flipMultiplier * 2 * scale, centerY - 6 * scale, 1 * scale, 1 * scale};
    SDL_Rect rightEye = {centerX + flipMultiplier * 1 * scale, centerY - 6 * scale, 1 * scale, 1 * scale};
    SDL_RenderFillRect(renderer, &leftEye);
    SDL_RenderFillRect(renderer, &rightEye);
    
    
    setColor(swordHandle);
    SDL_Rect swordHandle_rect = {centerX + flipMultiplier * 2 * scale, centerY - 2 * scale, 3 * scale, 8 * scale};
    SDL_RenderFillRect(renderer, &swordHandle_rect);
    
    setColor(swordGray);
   
    SDL_Rect blade = {centerX + flipMultiplier * 4 * scale, centerY - 8 * scale, 2 * scale, 20 * scale};
    SDL_RenderFillRect(renderer, &blade);
  
    SDL_Rect tip = {centerX + flipMultiplier * 5 * scale, centerY + 11 * scale, 1 * scale, 3 * scale};
    SDL_RenderFillRect(renderer, &tip);
    
    // Sword shine effect
    setColor({255, 255, 255, 200});
    SDL_Rect shine = {centerX + flipMultiplier * 4 * scale, centerY, 1 * scale, 6 * scale};
    SDL_RenderFillRect(renderer, &shine);

    // wind effect WIP
    if (player.isDashing || player.activeAbility != SamuraiAbility::NONE) {
        setColor(windColor);
      
        for (int i = 0; i < 360; i += 40) {
            float angle = i * 3.14159f / 180.0f;
            int windRadius = static_cast<int>(18 * cameraScale);
            int wx = centerX + static_cast<int>(std::cos(angle) * windRadius);
            int wy = centerY + static_cast<int>(std::sin(angle) * windRadius);
            SDL_Rect windParticle = {wx, wy, (std::max)(1, static_cast<int>(2 * cameraScale)), 
                                     (std::max)(1, static_cast<int>(4 * cameraScale))};
            SDL_RenderFillRect(renderer, &windParticle);
        }
        
      
        for (int i = 0; i < 3; i++) {
            int lineY = centerY - static_cast<int>(10 * cameraScale) + static_cast<int>(i * 8 * cameraScale);
            SDL_RenderDrawLine(renderer, 
                centerX - flipMultiplier * static_cast<int>(20 * cameraScale), lineY, 
                centerX - flipMultiplier * static_cast<int>(10 * cameraScale), lineY);
            SDL_RenderDrawLine(renderer, 
                centerX + flipMultiplier * static_cast<int>(10 * cameraScale), lineY, 
                centerX + flipMultiplier * static_cast<int>(20 * cameraScale), lineY);
        }
    }
    

    int barWidth = static_cast<int>(40 * cameraScale);
    int barHeight = static_cast<int>(5 * cameraScale);
    int barX = centerX - barWidth / 2;
    int barY = centerY - 25 * scale;
    
   
    SDL_Rect bgRect = {barX, barY, barWidth, barHeight};
    setColor(healthBarRed);
    SDL_RenderFillRect(renderer, &bgRect);
    
    
    float healthPercent = static_cast<float>(std::max(0, player.health)) / player.maxHealth;
    healthPercent = std::max(0.0f, std::min(1.0f, healthPercent)); 
    int fgWidth = static_cast<int>(barWidth * healthPercent);
    if (fgWidth > 0) {
        SDL_Rect fgRect = {barX, barY, fgWidth, barHeight};
        setColor(healthBarGreen);
        SDL_RenderFillRect(renderer, &fgRect);
    }
    
   
    
    setColor(textColor);
    int nameSize = static_cast<int>(12 * cameraScale);
    int nameWidth = static_cast<int>(player.name.length()) * nameSize / 2;
    int nameX = centerX - nameWidth / 2;
    int nameY = barY - nameSize - static_cast<int>(4 * cameraScale);
    renderText(player.name.c_str(), nameX, nameY, nameSize);
    
   
    if (isLocal) {
        
        for (int i = 0; i < player.qStacks; i++) {
            SDL_Rect stackRect = {centerX - 15 + i * 12, centerY + 25, 10, 3};
            setColor(windColor);
            SDL_RenderFillRect(renderer, &stackRect);
        }
    }
}

void Renderer::renderEnemy(const Enemy& enemy) {
    if (!enemy.isAlive) return;
    
    int screenX, screenY;
    worldToScreen(enemy.x, enemy.y, screenX, screenY);
    

    SDL_Color bodyOrange = {255, 120, 50, 255};    
    SDL_Color bodyDark = {200, 80, 30, 255};         
    SDL_Color eyeWhite = {255, 255, 255, 255};      
    SDL_Color eyeBlack = {20, 20, 20, 255};          
    SDL_Color wingRed = {220, 60, 40, 255};          
    SDL_Color accentYellow = {255, 200, 80, 255};    
    SDL_Color healthBarBg = {60, 60, 60, 255};
    SDL_Color healthBarRed = {200, 50, 50, 255};
    
    int scale = static_cast<int>(cameraScale);
    int centerX = screenX;
    int centerY = screenY;
    
   
    setColor(wingRed);
   
    SDL_Rect leftWing = {centerX - 12 * scale, centerY - 6 * scale, 5 * scale, 8 * scale};
    SDL_RenderFillRect(renderer, &leftWing);
  
    SDL_Rect rightWing = {centerX + 7 * scale, centerY - 6 * scale, 5 * scale, 8 * scale};
    SDL_RenderFillRect(renderer, &rightWing);
    

    setColor(bodyOrange);
    int bodyRadius = 8 * scale;
    for (int y = -bodyRadius; y <= bodyRadius; y++) {
        int width = static_cast<int>(std::sqrt(bodyRadius * bodyRadius - y * y));
        SDL_RenderDrawLine(renderer, 
            centerX - width, centerY + y,
            centerX + width, centerY + y);
    }
    
   
    setColor(bodyDark);
    for (int y = 2 * scale; y <= bodyRadius; y++) {
        int width = static_cast<int>(std::sqrt(bodyRadius * bodyRadius - y * y));
        SDL_RenderDrawLine(renderer, 
            centerX - width, centerY + y,
            centerX + width, centerY + y);
    }
    
 
    setColor(eyeWhite);
    int eyeRadius = 3 * scale;

    int leftEyeX = centerX - 4 * scale;
    int eyeY = centerY - 2 * scale;
    for (int y = -eyeRadius; y <= eyeRadius; y++) {
        int width = static_cast<int>(std::sqrt(eyeRadius * eyeRadius - y * y));
        SDL_RenderDrawLine(renderer, 
            leftEyeX - width, eyeY + y,
            leftEyeX + width, eyeY + y);
    }

    int rightEyeX = centerX + 4 * scale;
    for (int y = -eyeRadius; y <= eyeRadius; y++) {
        int width = static_cast<int>(std::sqrt(eyeRadius * eyeRadius - y * y));
        SDL_RenderDrawLine(renderer, 
            rightEyeX - width, eyeY + y,
            rightEyeX + width, eyeY + y);
    }
    
    // Draw pupils (black dots)
    setColor(eyeBlack);
    int pupilRadius = 1 * scale + 1;
    // Left pupil
    for (int y = -pupilRadius; y <= pupilRadius; y++) {
        int width = static_cast<int>(std::sqrt(pupilRadius * pupilRadius - y * y));
        SDL_RenderDrawLine(renderer, 
            leftEyeX - width, eyeY + y,
            leftEyeX + width, eyeY + y);
    }
    // Right pupil
    for (int y = -pupilRadius; y <= pupilRadius; y++) {
        int width = static_cast<int>(std::sqrt(pupilRadius * pupilRadius - y * y));
        SDL_RenderDrawLine(renderer, 
            rightEyeX - width, eyeY + y,
            rightEyeX + width, eyeY + y);
    }
    
    // Add yellow accent dots on body
    setColor(accentYellow);
    SDL_Rect dot1 = {centerX - 2 * scale, centerY + 4 * scale, 2 * scale, 2 * scale};
    SDL_RenderFillRect(renderer, &dot1);
    SDL_Rect dot2 = {centerX + 1 * scale, centerY + 5 * scale, 2 * scale, 2 * scale};
    SDL_RenderFillRect(renderer, &dot2);
    

    int barWidth = static_cast<int>(40 * cameraScale);
    int barHeight = static_cast<int>(5 * cameraScale);
    int barX = centerX - barWidth / 2;
    int barY = centerY - static_cast<int>(18 * cameraScale);
    
    SDL_Rect bgRect = {barX, barY, barWidth, barHeight};
    setColor(healthBarBg);
    SDL_RenderFillRect(renderer, &bgRect);
    
    
    float healthPercent = static_cast<float>(std::max(0, enemy.health)) / enemy.maxHealth;
    healthPercent = std::max(0.0f, std::min(1.0f, healthPercent)); 
    int fgWidth = static_cast<int>(barWidth * healthPercent);
    if (fgWidth > 0) {
        SDL_Rect fgRect = {barX, barY, fgWidth, barHeight};
        setColor(healthBarRed);
        SDL_RenderFillRect(renderer, &fgRect);
    }
}

void Renderer::renderProjectile(const Projectile& projectile) {

    int screenX, screenY;
    worldToScreen(projectile.x, projectile.y, screenX, screenY);
    
    int scale = static_cast<int>(cameraScale);
    
    if (projectile.isTornado) {
        int tornadoRadius = static_cast<int>(projectile.size * cameraScale);
        
        if (tornadoRadius < 5) tornadoRadius = 5;
        if (tornadoRadius > 200) tornadoRadius = 200;
        
        float time = SDL_GetTicks() / 150.0f; 
      
        SDL_Color darkBlue = {40, 100, 180, 255};      
        SDL_Color brightBlue = {100, 180, 255, 255};   
        SDL_Color lightBlue = {150, 210, 255, 200};    
        SDL_Color whiteCore = {200, 230, 255, 255};    
        
        int numSpirals = 5; 
        int numSegments = 30; 
        
        for (int spiralIdx = 0; spiralIdx < numSpirals; spiralIdx++) {
            float spiralOffset = (spiralIdx * 2.0f * 3.14159f / numSpirals) + time;
            
            for (int segment = 0; segment < numSegments; segment++) {
                float t = static_cast<float>(segment) / numSegments;
                float radius = t * tornadoRadius;
                
                // Create spiral curve
                float angle = spiralOffset + (t * 6.0f * 3.14159f); 
                
                int x = screenX + static_cast<int>(std::cos(angle) * radius);
                int y = screenY + static_cast<int>(std::sin(angle) * radius);
                
                // Gradient color based on distance from center
                SDL_Color spiralColor;
                if (t < 0.3f) {
                    spiralColor = whiteCore;
                } else if (t < 0.6f) {
                    spiralColor = brightBlue;
                } else {
                    spiralColor = darkBlue;
                }
                
                setColor(spiralColor);
                
                // Draw thicker spiral lines
                int thickness = static_cast<int>((1.0f - t) * 3 * scale);
                if (thickness < 1) thickness = 1;
                
                for (int thick = -thickness; thick <= thickness; thick++) {
                    SDL_RenderDrawPoint(renderer, x + thick, y);
                    SDL_RenderDrawPoint(renderer, x, y + thick);
                }
            }
        }
        
        // Draw circular rings at various radii for depth
        for (int ring = 1; ring <= 3; ring++) {
            int ringRadius = static_cast<int>(tornadoRadius * ring / 3.5f);
            float ringAngleOffset = time * (1.5f - ring * 0.3f); 
            
            SDL_Color ringColor;
            if (ring == 1) ringColor = lightBlue;
            else if (ring == 2) ringColor = brightBlue;
            else ringColor = darkBlue;
            
            setColor(ringColor);
            
            // Draw dashed ring with rotation
            int numDashes = 12 + ring * 4;
            for (int dash = 0; dash < numDashes; dash++) {
                float angle = ringAngleOffset + (dash * 2.0f * 3.14159f / numDashes);
                int x = screenX + static_cast<int>(std::cos(angle) * ringRadius);
                int y = screenY + static_cast<int>(std::sin(angle) * ringRadius);
                
                // Draw small dash
                int dashLen = 2 * scale;
                SDL_RenderDrawLine(renderer, x - dashLen, y, x + dashLen, y);
                SDL_RenderDrawLine(renderer, x, y - dashLen, x, y + dashLen);
            }
        }
        
        // Add particle effects around the tornado
        int numParticles = 20;
        for (int i = 0; i < numParticles; i++) {
            float particleAngle = time * 2.0f + (i * 2.0f * 3.14159f / numParticles);
            float particleRadius = tornadoRadius * 1.1f + std::sin(time * 3.0f + i) * tornadoRadius * 0.2f;
            
            int px = screenX + static_cast<int>(std::cos(particleAngle) * particleRadius);
            int py = screenY + static_cast<int>(std::sin(particleAngle) * particleRadius);
            
            setColor(lightBlue);
            SDL_RenderDrawPoint(renderer, px, py);
            SDL_RenderDrawPoint(renderer, px + 1, py);
            SDL_RenderDrawPoint(renderer, px, py + 1);
        }
        
        // Draw bright center core
        setColor(whiteCore);
        int centerSize = static_cast<int>(tornadoRadius * 0.15f);
        for (int y = -centerSize; y <= centerSize; y++) {
            int width = static_cast<int>(std::sqrt(centerSize * centerSize - y * y));
            SDL_RenderDrawLine(renderer, screenX - width, screenY + y, screenX + width, screenY + y);
        }
        
    } else if (projectile.isEnemyProjectile) {
        
        SDL_Color laserCore = {255, 50, 50, 255};     
        SDL_Color laserGlow = {255, 100, 80, 180};     
        SDL_Color laserOuter = {255, 150, 120, 100};   
        
        float angle = std::atan2(projectile.vy, projectile.vx);
        
        int laserLength = static_cast<int>(projectile.size * 1.5f * cameraScale);
        int laserWidth = static_cast<int>(projectile.size * 0.3f * cameraScale);
        
        float cosA = std::cos(angle);
        float sinA = std::sin(angle);
        
        setColor(laserOuter);
        for (int thick = -laserWidth * 2; thick <= laserWidth * 2; thick++) {
            int startX = screenX - static_cast<int>(cosA * laserLength / 2);
            int startY = screenY - static_cast<int>(sinA * laserLength / 2);
            int endX = screenX + static_cast<int>(cosA * laserLength / 2);
            int endY = screenY + static_cast<int>(sinA * laserLength / 2);
            
            int offsetX = static_cast<int>(-sinA * thick);
            int offsetY = static_cast<int>(cosA * thick);
            
            SDL_RenderDrawLine(renderer, 
                startX + offsetX, startY + offsetY,
                endX + offsetX, endY + offsetY);
        }
        
        setColor(laserGlow);
        for (int thick = -laserWidth; thick <= laserWidth; thick++) {
            int startX = screenX - static_cast<int>(cosA * laserLength / 2);
            int startY = screenY - static_cast<int>(sinA * laserLength / 2);
            int endX = screenX + static_cast<int>(cosA * laserLength / 2);
            int endY = screenY + static_cast<int>(sinA * laserLength / 2);
            
            int offsetX = static_cast<int>(-sinA * thick);
            int offsetY = static_cast<int>(cosA * thick);
            
            SDL_RenderDrawLine(renderer, 
                startX + offsetX, startY + offsetY,
                endX + offsetX, endY + offsetY);
        }
        
        setColor(laserCore);
        int startX = screenX - static_cast<int>(cosA * laserLength / 2);
        int startY = screenY - static_cast<int>(sinA * laserLength / 2);
        int endX = screenX + static_cast<int>(cosA * laserLength / 2);
        int endY = screenY + static_cast<int>(sinA * laserLength / 2);
        
        for (int i = -1; i <= 1; i++) {
            SDL_RenderDrawLine(renderer, 
                startX + i, startY, endX + i, endY);
            SDL_RenderDrawLine(renderer, 
                startX, startY + i, endX, endY + i);
        }
        
    } else {
        // Render normal Q blade 
        SDL_Color bladeColor = {240, 240, 250, 255};     
        SDL_Color bladeEdge = {200, 200, 220, 255};      
        
        // Calculate blade rotation based on velocity direction
        float angle = std::atan2(projectile.vy, projectile.vx);
        
        // Blade dimensions 
        int bladeLength = static_cast<int>(projectile.size * 2 * cameraScale);
        int bladeWidth = static_cast<int>(projectile.size * 0.5f * cameraScale);
        
        // Calculate blade corners 
        float cosA = std::cos(angle);
        float sinA = std::sin(angle);
        
        // Four corners of the blade
        SDL_Point points[5];
        float halfLen = bladeLength / 2.0f;
        float halfWidth = bladeWidth / 2.0f;
        
        // Front-right
        points[0].x = screenX + static_cast<int>(cosA * halfLen - sinA * halfWidth);
        points[0].y = screenY + static_cast<int>(sinA * halfLen + cosA * halfWidth);
        
        // Front-left
        points[1].x = screenX + static_cast<int>(cosA * halfLen + sinA * halfWidth);
        points[1].y = screenY + static_cast<int>(sinA * halfLen - cosA * halfWidth);
        
        // Back-left
        points[2].x = screenX + static_cast<int>(-cosA * halfLen + sinA * halfWidth);
        points[2].y = screenY + static_cast<int>(-sinA * halfLen - cosA * halfWidth);
        
        // Back-right
        points[3].x = screenX + static_cast<int>(-cosA * halfLen - sinA * halfWidth);
        points[3].y = screenY + static_cast<int>(-sinA * halfLen + cosA * halfWidth);
        
        // Close the polygon
        points[4] = points[0];
        
        // Fill the blade 
        setColor(bladeColor);
        for (int i = 0; i < 4; i++) {
            SDL_RenderDrawLine(renderer, points[i].x, points[i].y, points[i+1].x, points[i+1].y);
        }
        
        // Draw filled blade by scanning through the polygon area
        int minY = points[0].y, maxY = points[0].y;
        for (int i = 1; i < 4; i++) {
            minY = std::min(minY, points[i].y);
            maxY = std::max(maxY, points[i].y);
        }
        
        for (int y = minY; y <= maxY; y++) {
            int minX = screenX + bladeLength, maxX = screenX - bladeLength;
            
            // Find intersections with polygon edges at this y coordinate
            for (int i = 0; i < 4; i++) {
                int y1 = points[i].y, y2 = points[i+1].y;
                if ((y1 <= y && y < y2) || (y2 <= y && y < y1)) {
                    int x1 = points[i].x, x2 = points[i+1].x;
                    int x = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
                    minX = std::min(minX, x);
                    maxX = std::max(maxX, x);
                }
            }
            
            if (minX <= maxX) {
                SDL_RenderDrawLine(renderer, minX, y, maxX, y);
            }
        }
        
        // Draw edge outline
        setColor(bladeEdge);
        for (int i = 0; i < 4; i++) {
            SDL_RenderDrawLine(renderer, points[i].x, points[i].y, points[i+1].x, points[i+1].y);
        }
    }
}

void Renderer::renderWindWall(const Player& player) {
    if (!player.hasWindWall) return;
    
    // Convert world position to screen coordinates
    int screenX, screenY;
    worldToScreen(player.x, player.y, screenX, screenY);
    
    int wallRadius = static_cast<int>(player.windWallRadius * cameraScale);
    float time = SDL_GetTicks() / 200.0f; 
    
    // Define wind colors
    SDL_Color windCore = {150, 220, 255, 200};     
    SDL_Color windOuter = {100, 180, 255, 120};    
    SDL_Color windParticle = {200, 240, 255, 180}; 
    
    // Draw effect rings
    for (int ring = 3; ring >= 1; ring--) {
        int ringRadius = static_cast<int>(wallRadius * ring / 3.0f);
        float ringAlpha = 255.0f / (ring + 1); 
        
        SDL_Color ringColor = windOuter;
        ringColor.a = static_cast<Uint8>(ringAlpha);
        setColor(ringColor);
        
        // Draw animated dashed circle
        int numSegments = 24 + ring * 6;
        float angleOffset = time * (1.0f + ring * 0.3f); 
        
        for (int i = 0; i < numSegments; i++) {
           
            if (i % 2 == 0) {
                float angle = angleOffset + (i * 2.0f * 3.14159f / numSegments);
                int x = screenX + static_cast<int>(std::cos(angle) * ringRadius);
                int y = screenY + static_cast<int>(std::sin(angle) * ringRadius);
                
                // Draw small wind segment
                int segmentSize = 2 + ring;
                for (int dx = -segmentSize; dx <= segmentSize; dx++) {
                    for (int dy = -segmentSize; dy <= segmentSize; dy++) {
                        if (dx*dx + dy*dy <= segmentSize*segmentSize) {
                            SDL_RenderDrawPoint(renderer, x + dx, y + dy);
                        }
                    }
                }
            }
        }
    }
    
    // Swirling
    setColor(windParticle);
    int numParticles = 30;
    for (int i = 0; i < numParticles; i++) {
        float particleAngle = time * 2.5f + (i * 2.0f * 3.14159f / numParticles);
        float particleRadius = wallRadius * 0.7f + std::sin(time * 4.0f + i * 0.5f) * wallRadius * 0.2f;
        
        int px = screenX + static_cast<int>(std::cos(particleAngle) * particleRadius);
        int py = screenY + static_cast<int>(std::sin(particleAngle) * particleRadius);
        
        //tail effect
        SDL_RenderDrawPoint(renderer, px, py);
        SDL_RenderDrawPoint(renderer, px - 1, py);
        SDL_RenderDrawPoint(renderer, px, py - 1);
    }
    
    // pulsing effect
    setColor(windCore);
    float pulseSize = 1.0f + 0.3f * std::sin(time * 5.0f);
    int coreRadius = static_cast<int>(wallRadius * 0.3f * pulseSize);
    
    // core circle
    for (int y = -coreRadius; y <= coreRadius; y++) {
        int width = static_cast<int>(std::sqrt(coreRadius * coreRadius - y * y));
        SDL_RenderDrawLine(renderer, screenX - width, screenY + y, screenX + width, screenY + y);
    }
    
    // Radial rays
    setColor({180, 230, 255, 150});
    int numRays = 8;
    for (int ray = 0; ray < numRays; ray++) {
        float rayAngle = time + (ray * 2.0f * 3.14159f / numRays);
        int innerRadius = static_cast<int>(wallRadius * 0.4f);
        int outerRadius = static_cast<int>(wallRadius * 0.9f);
        
        int x1 = screenX + static_cast<int>(std::cos(rayAngle) * innerRadius);
        int y1 = screenY + static_cast<int>(std::sin(rayAngle) * innerRadius);
        int x2 = screenX + static_cast<int>(std::cos(rayAngle) * outerRadius);
        int y2 = screenY + static_cast<int>(std::sin(rayAngle) * outerRadius);
        
        SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
    }
}

void Renderer::renderText(const char* text, int x, int y, int size) {
    
    setColor(textColor);
    int textWidth = static_cast<int>(std::strlen(text)) * size / 2;
    SDL_Rect rect = {x, y, textWidth, size};
    SDL_RenderDrawRect(renderer, &rect);
}

void Renderer::renderUI(uint32_t playerId, int playerCount, int fps) {
    setColor(textColor);
    
    // Player ID indicator 
    SDL_Rect idRect = {10, 10, 150, 25};
    SDL_RenderDrawRect(renderer, &idRect);
    
    // Player count
    SDL_Rect countRect = {10, 40, 150, 25};
    SDL_RenderDrawRect(renderer, &countRect);
    
    // FPS counter 
    SDL_Rect fpsRect = {width - 80, 10, 70, 25};
    SDL_RenderDrawRect(renderer, &fpsRect);
    
    // Controls info 
    const char* controls[] = {
        "Right-click: Move",
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

void Renderer::renderSwordSwing(float originX, float originY, float rotationRad,
                                float arcDegrees, float range) {
   
    int cx, cy;
    worldToScreen(originX, originY, cx, cy);

    SDL_Color inner = {150, 220, 255, 200};
    SDL_Color mid = {100, 180, 255, 180};
    SDL_Color outer = {60, 140, 220, 160};

    float halfArcRad = (arcDegrees * 0.5f) * 3.14159f / 180.0f;
    float startAngle = rotationRad - halfArcRad;
    float endAngle = rotationRad + halfArcRad;

 
    const int bands = 3;
    for (int b = 0; b < bands; ++b) {
        float t = static_cast<float>(b) / (bands - 1);
        float bandRange = range * (0.8f + 0.2f * t);
        SDL_Color col = outer;
        if (b == 1) col = mid; else if (b == 2) col = inner;
        setColor(col);

        int segments = 24;
        int prevX = 0, prevY = 0;
        bool hasPrev = false;
        for (int i = 0; i <= segments; ++i) {
            float a = startAngle + (endAngle - startAngle) * (static_cast<float>(i) / segments);
            int x = cx + static_cast<int>(std::cos(a) * bandRange * cameraScale);
            int y = cy + static_cast<int>(std::sin(a) * bandRange * cameraScale);
            if (hasPrev) {
                SDL_RenderDrawLine(renderer, prevX, prevY, x, y);
            }
            prevX = x; prevY = y; hasPrev = true;
        }

        int radialLines = 6;
        for (int r = 0; r < radialLines; ++r) {
            float a = startAngle + (endAngle - startAngle) * (static_cast<float>(r) / (radialLines - 1));
            int x1 = cx + static_cast<int>(std::cos(a) * (bandRange * 0.5f) * cameraScale);
            int y1 = cy + static_cast<int>(std::sin(a) * (bandRange * 0.5f) * cameraScale);
            int x2 = cx + static_cast<int>(std::cos(a) * bandRange * cameraScale);
            int y2 = cy + static_cast<int>(std::sin(a) * bandRange * cameraScale);
            SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
        }
    }
}

void Renderer::renderDashBurst(float originX, float originY, float rotationRad, float length) {
    int cx, cy;
    worldToScreen(originX, originY, cx, cy);

    SDL_Color c1 = {150, 220, 255, 200};
    SDL_Color c2 = {100, 180, 255, 160};
    SDL_Color c3 = {60, 140, 220, 140};

    float cosA = std::cos(rotationRad);
    float sinA = std::sin(rotationRad);
    float l = length * cameraScale;

    auto drawBurstLine = [&](float offsetY, const SDL_Color& col){
        setColor(col);
        int x1 = cx + static_cast<int>(cosA * 10.0f * cameraScale - sinA * offsetY);
        int y1 = cy + static_cast<int>(sinA * 10.0f * cameraScale + cosA * offsetY);
        int x2 = cx + static_cast<int>(cosA * l - sinA * offsetY);
        int y2 = cy + static_cast<int>(sinA * l + cosA * offsetY);
        SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
    };

    drawBurstLine(-6.0f, c1);
    drawBurstLine(0.0f, c2);
    drawBurstLine(6.0f, c3);
}

void Renderer::renderRTornado(float x, float y) {
    int screenX, screenY;
    worldToScreen(x, y, screenX, screenY);

    SDL_Color windCore = {100, 180, 255, 220};
    SDL_Color windOuter = {150, 220, 255, 160};
    SDL_Color windGlow = {200, 240, 255, 100};
    
    int tornadoSize = static_cast<int>(Config::R_TORNADO_SIZE * cameraScale);
   
    if (tornadoSize < 10) tornadoSize = 10;
    if (tornadoSize > 200) tornadoSize = 200;

    setColor(windGlow);
    for (int r = tornadoSize + 8; r >= tornadoSize + 2; r -= 2) {
        renderCircle(screenX, screenY, r);
    }
    
    setColor(windOuter);
    for (int r = tornadoSize; r >= tornadoSize - 4; r--) {
        if (r > 0) renderCircle(screenX, screenY, r);
    }
    
    setColor(windCore);
    int minR = tornadoSize - 10;
    if (minR < 1) minR = 1;
    for (int r = tornadoSize - 6; r >= minR; r--) {
        if (r > 0) renderCircle(screenX, screenY, r);
    }

    auto now = std::chrono::steady_clock::now();
    float time = std::chrono::duration<float>(now.time_since_epoch()).count();
    float angle = time * 3.0f; 
    
    setColor(windCore);
    for (int i = 0; i < 3; i++) {
        float swirlAngle = angle + (i * 2.0f * 3.14159f / 3.0f);
        int x1 = screenX + static_cast<int>(std::cos(swirlAngle) * (tornadoSize - 8));
        int y1 = screenY + static_cast<int>(std::sin(swirlAngle) * (tornadoSize - 8));
        int x2 = screenX + static_cast<int>(std::cos(swirlAngle) * tornadoSize);
        int y2 = screenY + static_cast<int>(std::sin(swirlAngle) * tornadoSize);
        SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
    }
}
