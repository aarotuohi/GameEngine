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

    
    if (player.activeAbility != SamuraiAbility::NONE) {
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
    
    
    float sizeMultiplier = enemy.isBoss ? 2.0f : 1.0f;

    SDL_Color bodyOrange = {255, 120, 50, 255};    
    SDL_Color bodyDark = {200, 80, 30, 255};         
    SDL_Color eyeWhite = {255, 255, 255, 255};      
    SDL_Color eyeBlack = {20, 20, 20, 255};          
    SDL_Color wingRed = {220, 60, 40, 255};          
    SDL_Color accentYellow = {255, 200, 80, 255};    
    SDL_Color healthBarBg = {60, 60, 60, 255};
    SDL_Color healthBarRed = {200, 50, 50, 255};
    SDL_Color bossGlow = {255, 50, 255, 180}; 
    
    int scale = static_cast<int>(cameraScale * sizeMultiplier);
    int centerX = screenX;
    int centerY = screenY;
    

    if (enemy.isBoss) {
        setColor(bossGlow);
        int glowRadius = static_cast<int>(18 * cameraScale * sizeMultiplier);
        for (int y = -glowRadius; y <= glowRadius; y++) {
            int width = static_cast<int>(std::sqrt(glowRadius * glowRadius - y * y));
            SDL_RenderDrawLine(renderer, 
                centerX - width, centerY + y,
                centerX + width, centerY + y);
        }
    }
   
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
    

    int barWidth = static_cast<int>(40 * cameraScale * sizeMultiplier);
    int barHeight = static_cast<int>(5 * cameraScale);
    int barX = centerX - barWidth / 2;
    int barY = centerY - static_cast<int>(18 * cameraScale * sizeMultiplier);
    
    SDL_Rect bgRect = {barX, barY, barWidth, barHeight};
    setColor(healthBarBg);
    SDL_RenderFillRect(renderer, &bgRect);
    
    
    float healthPercent = static_cast<float>(std::max(0, enemy.health)) / enemy.maxHealth;
    healthPercent = std::max(0.0f, std::min(1.0f, healthPercent)); 
    int fgWidth = static_cast<int>(barWidth * healthPercent);
    if (fgWidth > 0) {
        SDL_Rect fgRect = {barX, barY, fgWidth, barHeight};
        setColor(enemy.isBoss ? bossGlow : healthBarRed);
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

void Renderer::renderUI(uint32_t playerId, int playerCount, int fps, int kills, int wave) {
    setColor(textColor);
    
    // Player ID indicator 
    SDL_Rect idRect = {10, 10, 150, 25};
    SDL_RenderDrawRect(renderer, &idRect);
    
    // Player count
    SDL_Rect countRect = {10, 40, 150, 25};
    SDL_RenderDrawRect(renderer, &countRect);
    
 
    SDL_Color waveBg = {40, 40, 60, 220};
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    setColor(waveBg);
    SDL_Rect waveBgRect = {width - 160, 10, 150, 40};
    SDL_RenderFillRect(renderer, &waveBgRect);
    
    SDL_Color waveBorder = {100, 150, 200, 255};
    setColor(waveBorder);
    SDL_RenderDrawRect(renderer, &waveBgRect);
    SDL_Rect waveBorderInner = {waveBgRect.x + 1, waveBgRect.y + 1, waveBgRect.w - 2, waveBgRect.h - 2};
    SDL_RenderDrawRect(renderer, &waveBorderInner);
    
 
    SDL_Color waveTextColor = {150, 200, 255, 255};
    setColor(waveTextColor);
    int waveTextX = width - 145;
    int waveTextY = 20;
    int waveLetterSize = 2;
    
   
    SDL_Rect w[] = {{waveTextX, waveTextY, waveLetterSize, 16}, 
                    {waveTextX + 5, waveTextY + 10, waveLetterSize, 6}, 
                    {waveTextX + 10, waveTextY, waveLetterSize, 16}};
    for (auto& r : w) SDL_RenderFillRect(renderer, &r);
    waveTextX += 16;
    
 
    SDL_Rect a[] = {{waveTextX, waveTextY, 10, waveLetterSize}, 
                    {waveTextX, waveTextY, waveLetterSize, 16}, 
                    {waveTextX + 8, waveTextY, waveLetterSize, 16}, 
                    {waveTextX, waveTextY + 7, 10, waveLetterSize}};
    for (auto& r : a) SDL_RenderFillRect(renderer, &r);
    waveTextX += 14;
    

    SDL_Rect v[] = {{waveTextX, waveTextY, waveLetterSize, 12}, 
                    {waveTextX + 4, waveTextY + 12, waveLetterSize, 4}, 
                    {waveTextX + 8, waveTextY, waveLetterSize, 12}};
    for (auto& r : v) SDL_RenderFillRect(renderer, &r);
    waveTextX += 14;
    

    SDL_Rect e[] = {{waveTextX, waveTextY, waveLetterSize, 16}, 
                    {waveTextX, waveTextY, 10, waveLetterSize}, 
                    {waveTextX, waveTextY + 7, 8, waveLetterSize}, 
                    {waveTextX, waveTextY + 14, 10, waveLetterSize}};
    for (auto& r : e) SDL_RenderFillRect(renderer, &r);
    waveTextX += 14;
    

    SDL_Rect waveColon[] = {{waveTextX, waveTextY + 4, waveLetterSize, waveLetterSize}, 
                             {waveTextX, waveTextY + 10, waveLetterSize, waveLetterSize}};
    for (auto& r : waveColon) SDL_RenderFillRect(renderer, &r);
    waveTextX += 6;
    

    SDL_Color waveNumberColor = {255, 255, 255, 255};
    setColor(waveNumberColor);
    if (wave >= 10) {
        drawDigit(waveTextX, waveTextY, wave / 10);
        waveTextX += 14;
        drawDigit(waveTextX, waveTextY, wave % 10);
    } else {
        drawDigit(waveTextX, waveTextY, wave);
    }
 
    SDL_Color killBg = {40, 40, 40, 220};
    setColor(killBg);
    SDL_Rect killBgRect = {width/2 - 100, 10, 200, 40};
    SDL_RenderFillRect(renderer, &killBgRect);
    
    SDL_Color killBorder = {200, 150, 50, 255};
    setColor(killBorder);
    SDL_RenderDrawRect(renderer, &killBgRect);
    SDL_Rect killBorderInner = {killBgRect.x + 1, killBgRect.y + 1, killBgRect.w - 2, killBgRect.h - 2};
    SDL_RenderDrawRect(renderer, &killBorderInner);
    
  
    SDL_Color killTextColor = {255, 200, 100, 255};
    setColor(killTextColor);
    int textX = width/2 - 80;
    int textY = 20;
    

    SDL_Rect k[] = {
        {textX, textY, 2, 16},          
        {textX + 2, textY + 7, 2, 2},   
        {textX + 4, textY + 5, 2, 2},   
        {textX + 6, textY + 3, 2, 2},   
        {textX + 8, textY + 1, 2, 2},   
        {textX + 4, textY + 9, 2, 2},   
        {textX + 6, textY + 11, 2, 2},  
        {textX + 8, textY + 13, 2, 2}   
    };
    for (auto& r : k) SDL_RenderFillRect(renderer, &r);
    textX += 14;
 
    SDL_Rect i[] = {{textX, textY, 8, 2}, {textX + 3, textY, 2, 16}, {textX, textY + 14, 8, 2}};
    for (auto& r : i) SDL_RenderFillRect(renderer, &r);
    textX += 12;
    
    SDL_Rect l[] = {{textX, textY, 2, 16}, {textX, textY + 14, 10, 2}};
    for (auto& r : l) SDL_RenderFillRect(renderer, &r);
    textX += 14;
    
    SDL_Rect l2[] = {{textX, textY, 2, 16}, {textX, textY + 14, 10, 2}};
    for (auto& r : l2) SDL_RenderFillRect(renderer, &r);
    textX += 14;
    
    SDL_Rect s[] = {{textX, textY, 10, 2}, {textX, textY, 2, 8}, {textX, textY + 7, 10, 2}, {textX + 8, textY + 7, 2, 8}, {textX, textY + 14, 10, 2}};
    for (auto& r : s) SDL_RenderFillRect(renderer, &r);
    textX += 14;
    
    SDL_Rect colon[] = {{textX, textY + 4, 2, 2}, {textX, textY + 10, 2, 2}};
    for (auto& r : colon) SDL_RenderFillRect(renderer, &r);
    textX += 8;
    
    SDL_Color numberColor = {255, 255, 255, 255};
    setColor(numberColor);
    
    if (kills >= 100) {
        drawDigit(textX, textY, kills / 100);
        textX += 14;
        drawDigit(textX, textY, (kills / 10) % 10);
        textX += 14;
        drawDigit(textX, textY, kills % 10);
    } else if (kills >= 10) {
        drawDigit(textX, textY, kills / 10);
        textX += 14;
        drawDigit(textX, textY, kills % 10);
    } else {
        drawDigit(textX, textY, kills);
    }
    
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    
    
    setColor(textColor);
    SDL_Rect fpsRect = {width - 80, 10, 70, 25};
    SDL_RenderDrawRect(renderer, &fpsRect);
    
    // Controls info 
    const char* controls[] = {
        "Right-click: Move",
        "Mouse: Aim",
        "ESC: Quit"
        "R: Restarts"
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

void Renderer::renderShockwave(float centerX, float centerY, float progress, float maxRadius) {
    int screenX, screenY;
    worldToScreen(centerX, centerY, screenX, screenY);

    float currentRadius = maxRadius * progress * cameraScale;

    int alpha = static_cast<int>(255 * (1.0f - progress));
    if (alpha < 0) alpha = 0;
    if (alpha > 255) alpha = 255;
    
    
    float ringThickness = 15.0f * cameraScale;
    
    SDL_Color outerEdge = {120, 200, 255, static_cast<Uint8>(alpha * 0.8f)};
    setColor(outerEdge);
    renderCircle(screenX, screenY, static_cast<int>(currentRadius));
    
    if (currentRadius > ringThickness * 0.5f) {
        SDL_Color middleLayer = {150, 220, 255, static_cast<Uint8>(alpha * 0.9f)};
        setColor(middleLayer);
        renderCircle(screenX, screenY, static_cast<int>(currentRadius - ringThickness * 0.3f));
    }
    
    if (currentRadius > ringThickness) {
        SDL_Color innerEdge = {180, 240, 255, static_cast<Uint8>(alpha)};
        setColor(innerEdge);
        renderCircle(screenX, screenY, static_cast<int>(currentRadius - ringThickness));
    }
    
    int numParticles = 16;
    for (int i = 0; i < numParticles; i++) {
        float angle = (i * 2.0f * 3.14159f / numParticles) + (progress * 3.0f); 
        
        int px = screenX + static_cast<int>(std::cos(angle) * currentRadius);
        int py = screenY + static_cast<int>(std::sin(angle) * currentRadius);
        

        float tangentAngle = angle + 3.14159f / 2.0f; 
        int streakLength = static_cast<int>(8.0f * cameraScale);
        
        int sx1 = px - static_cast<int>(std::cos(tangentAngle) * streakLength * 0.5f);
        int sy1 = py - static_cast<int>(std::sin(tangentAngle) * streakLength * 0.5f);
        int sx2 = px + static_cast<int>(std::cos(tangentAngle) * streakLength * 0.5f);
        int sy2 = py + static_cast<int>(std::sin(tangentAngle) * streakLength * 0.5f);
        
        SDL_Color windStreak = {200, 240, 255, static_cast<Uint8>(alpha * 0.8f)};
        setColor(windStreak);
        SDL_RenderDrawLine(renderer, sx1, sy1, sx2, sy2);
    }
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

void Renderer::renderGameOver() {
    
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_Color overlay = {0, 0, 0, 200};
    setColor(overlay);
    SDL_Rect fullScreen = {0, 0, width, height};
    SDL_RenderFillRect(renderer, &fullScreen);
    
   
    SDL_Color boxBg = {30, 30, 30, 255};
    setColor(boxBg);
    SDL_Rect box = {width/2 - 300, height/2 - 250, 600, 500};
    SDL_RenderFillRect(renderer, &box);
    
    
    SDL_Color border = {200, 50, 50, 255};
    setColor(border);
    for (int i = 0; i < 6; i++) {
        SDL_Rect borderRect = {box.x - i, box.y - i, box.w + i*2, box.h + i*2};
        SDL_RenderDrawRect(renderer, &borderRect);
    }
    
   
    SDL_Color titleColor = {255, 80, 80, 255};
    setColor(titleColor);
    
   
    int titleY = height/2 - 150;
    int letterWidth = 40;
    int letterHeight = 60;
    int spacing = 10;
    int totalWidth = letterWidth * 8 + spacing * 7; 
    int startX = width/2 - totalWidth/2;
    
 
    SDL_Rect gParts[] = {
        {startX, titleY, letterWidth, 10},
        {startX, titleY, 10, letterHeight},
        {startX, titleY + letterHeight - 10, letterWidth, 10},
        {startX + letterWidth - 10, titleY + letterHeight/2, 10, letterHeight/2},
        {startX + letterWidth/2, titleY + letterHeight/2, letterWidth/2, 10}
    };
    for (auto& p : gParts) SDL_RenderFillRect(renderer, &p);
    
    startX += letterWidth + spacing;
    
 
    SDL_Rect aParts[] = {
        {startX, titleY, letterWidth, 10},
        {startX, titleY, 10, letterHeight},
        {startX + letterWidth - 10, titleY, 10, letterHeight},
        {startX, titleY + letterHeight/2, letterWidth, 10}
    };
    for (auto& p : aParts) SDL_RenderFillRect(renderer, &p);
    
    startX += letterWidth + spacing;
    

    SDL_Rect mParts[] = {
        {startX, titleY, 10, letterHeight},
        {startX + letterWidth/2 - 5, titleY, 10, letterHeight/2},
        {startX + letterWidth - 10, titleY, 10, letterHeight}
    };
    for (auto& p : mParts) SDL_RenderFillRect(renderer, &p);
    
    startX += letterWidth + spacing;
    
  
    SDL_Rect eParts[] = {
        {startX, titleY, 10, letterHeight},
        {startX, titleY, letterWidth, 10},
        {startX, titleY + letterHeight/2 - 5, letterWidth - 10, 10},
        {startX, titleY + letterHeight - 10, letterWidth, 10}
    };
    for (auto& p : eParts) SDL_RenderFillRect(renderer, &p);
    
    startX += letterWidth + spacing + 20; 
    
    SDL_Rect oParts[] = {
        {startX, titleY, letterWidth, 10},
        {startX, titleY, 10, letterHeight},
        {startX + letterWidth - 10, titleY, 10, letterHeight},
        {startX, titleY + letterHeight - 10, letterWidth, 10}
    };
    for (auto& p : oParts) SDL_RenderFillRect(renderer, &p);
    
    startX += letterWidth + spacing;
    
 
    SDL_Rect vParts[] = {
        {startX, titleY, 10, letterHeight - 20},
        {startX + letterWidth - 10, titleY, 10, letterHeight - 20},
        {startX + letterWidth/2 - 5, titleY + letterHeight - 20, 10, 20}
    };
    for (auto& p : vParts) SDL_RenderFillRect(renderer, &p);
    
    startX += letterWidth + spacing;
    
 
    for (auto& p : eParts) {
        SDL_Rect shifted = {p.x + (startX - eParts[0].x), p.y, p.w, p.h};
        SDL_RenderFillRect(renderer, &shifted);
    }
    
    startX += letterWidth + spacing;
    

   
    SDL_Rect rParts[] = {
        {startX, titleY, 10, letterHeight}, 
        {startX, titleY, letterWidth - 5, 10},  
        {startX + letterWidth - 15, titleY, 10, letterHeight/2 + 5},  
        {startX, titleY + letterHeight/2 - 5, letterWidth - 10, 10},  
        {startX + letterWidth/2 - 5, titleY + letterHeight/2 + 5, 8, 8},  
        {startX + letterWidth/2 + 3, titleY + letterHeight/2 + 13, 8, 8},  
        {startX + letterWidth/2 + 11, titleY + letterHeight/2 + 21, 8, 8},
        {startX + letterWidth/2 + 19, titleY + letterHeight/2 + 29, 8, letterHeight/2 - 29}  
    };
    for (auto& p : rParts) SDL_RenderFillRect(renderer, &p);
    
   
    int buttonWidth = 300;
    int buttonHeight = 60;
    int button1Y = height/2 + 20;
    
    SDL_Color buttonColor = {80, 150, 80, 255};
    setColor(buttonColor);
    SDL_Rect startButton = {width/2 - buttonWidth/2, button1Y, buttonWidth, buttonHeight};
    SDL_RenderFillRect(renderer, &startButton);
    
    SDL_Color buttonBorder = {120, 200, 120, 255};
    setColor(buttonBorder);
    for (int i = 0; i < 3; i++) {
        SDL_Rect border = {startButton.x - i, startButton.y - i, startButton.w + i*2, startButton.h + i*2};
        SDL_RenderDrawRect(renderer, &border);
    }
    

    SDL_Color whiteText = {255, 255, 255, 255};
    setColor(whiteText);
    int textSize = 8;
    int textY = button1Y + buttonHeight/2 - textSize;
    int textStartX = width/2 - 110;
    
   
    SDL_Rect s1[] = {{textStartX, textY, 15, 3}, {textStartX, textY, 3, 8}, {textStartX, textY + 7, 15, 3}, 
                     {textStartX + 12, textY + 7, 3, 8}, {textStartX, textY + 13, 15, 3}};
    for (auto& p : s1) SDL_RenderFillRect(renderer, &p);
    textStartX += 20;
    
    SDL_Rect t1[] = {{textStartX, textY, 15, 3}, {textStartX + 6, textY, 3, 16}};
    for (auto& p : t1) SDL_RenderFillRect(renderer, &p);
    textStartX += 20;
    

    SDL_Rect a1[] = {{textStartX, textY, 15, 3}, {textStartX, textY, 3, 16}, {textStartX + 12, textY, 3, 16}, {textStartX, textY + 7, 15, 3}};
    for (auto& p : a1) SDL_RenderFillRect(renderer, &p);
    textStartX += 20;
    
    SDL_Rect r1[] = {
        {textStartX, textY, 3, 16},          
        {textStartX, textY, 15, 3},          
        {textStartX + 12, textY, 3, 8},      
        {textStartX, textY + 7, 15, 3},      
        {textStartX + 5, textY + 9, 3, 2},  
        {textStartX + 8, textY + 11, 3, 2},  
        {textStartX + 11, textY + 13, 3, 3}  
    };
    for (auto& p : r1) SDL_RenderFillRect(renderer, &p);
    textStartX += 20;
    

    SDL_Rect t2[] = {{textStartX, textY, 15, 3}, {textStartX + 6, textY, 3, 16}};
    for (auto& p : t2) SDL_RenderFillRect(renderer, &p);
    textStartX += 25;
    
  
    SDL_Rect g1[] = {{textStartX, textY, 15, 3}, {textStartX, textY, 3, 16}, {textStartX, textY + 13, 15, 3},
                     {textStartX + 12, textY + 7, 3, 9}, {textStartX + 7, textY + 7, 8, 3}};
    for (auto& p : g1) SDL_RenderFillRect(renderer, &p);
    textStartX += 20;
    

    SDL_Rect a2[] = {{textStartX, textY, 15, 3}, {textStartX, textY, 3, 16}, {textStartX + 12, textY, 3, 16}, {textStartX, textY + 7, 15, 3}};
    for (auto& p : a2) SDL_RenderFillRect(renderer, &p);
    textStartX += 20;
    
  
    SDL_Rect m1[] = {{textStartX, textY, 3, 16}, {textStartX + 6, textY, 3, 10}, {textStartX + 12, textY, 3, 16}};
    for (auto& p : m1) SDL_RenderFillRect(renderer, &p);
    textStartX += 20;
    

    SDL_Rect e1[] = {{textStartX, textY, 3, 16}, {textStartX, textY, 15, 3}, {textStartX, textY + 7, 12, 3}, {textStartX, textY + 13, 15, 3}};
    for (auto& p : e1) SDL_RenderFillRect(renderer, &p);
    
    
    int button2Y = button1Y + 90;
    
    SDL_Color quitButtonColor = {150, 80, 80, 255};
    setColor(quitButtonColor);
    SDL_Rect quitButton = {width/2 - buttonWidth/2, button2Y, buttonWidth, buttonHeight};
    SDL_RenderFillRect(renderer, &quitButton);
    
    SDL_Color quitBorder = {200, 120, 120, 255};
    setColor(quitBorder);
    for (int i = 0; i < 3; i++) {
        SDL_Rect border = {quitButton.x - i, quitButton.y - i, quitButton.w + i*2, quitButton.h + i*2};
        SDL_RenderDrawRect(renderer, &border);
    }
    
   
    setColor(whiteText);
    int quitTextY = button2Y + buttonHeight/2 - textSize;
    int quitTextX = width/2 - 40;
    
   
    SDL_Rect q1[] = {{quitTextX, quitTextY, 15, 3}, {quitTextX, quitTextY, 3, 16}, {quitTextX + 12, quitTextY, 3, 16},
                     {quitTextX, quitTextY + 13, 15, 3}, {quitTextX + 10, quitTextY + 11, 8, 3}};
    for (auto& p : q1) SDL_RenderFillRect(renderer, &p);
    quitTextX += 20;
    
    
    SDL_Rect u1[] = {{quitTextX, quitTextY, 3, 16}, {quitTextX + 12, quitTextY, 3, 16}, {quitTextX, quitTextY + 13, 15, 3}};
    for (auto& p : u1) SDL_RenderFillRect(renderer, &p);
    quitTextX += 20;
    
 
    SDL_Rect i1[] = {{quitTextX, quitTextY, 15, 3}, {quitTextX + 6, quitTextY, 3, 16}, {quitTextX, quitTextY + 13, 15, 3}};
    for (auto& p : i1) SDL_RenderFillRect(renderer, &p);
    quitTextX += 20;
    
  
    SDL_Rect t3[] = {{quitTextX, quitTextY, 15, 3}, {quitTextX + 6, quitTextY, 3, 16}};
    for (auto& p : t3) SDL_RenderFillRect(renderer, &p);
    
    
    SDL_Color grayText = {180, 180, 180, 255};
    setColor(grayText);
    SDL_Rect instructRect = {width/2 - 150, height/2 + 200, 300, 20};
    SDL_RenderDrawRect(renderer, &instructRect);
    
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

void Renderer::renderCooldowns(float qCooldown, float wCooldown, float eCooldown, float rCooldown) {
    
    int boxSize = 60;
    int spacing = 10;
    int startX = width/2 - (boxSize * 4 + spacing * 3) / 2;
    int startY = height - boxSize - 20;
    
    auto renderCooldownBox = [&](int x, int y, const char* key, float cooldown, SDL_Color keyColor) {
      
        SDL_Color boxBg = {40, 40, 40, 200};
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        setColor(boxBg);
        SDL_Rect bg = {x, y, boxSize, boxSize};
        SDL_RenderFillRect(renderer, &bg);
        
        
        SDL_Color border = {100, 100, 100, 255};
        setColor(border);
        SDL_RenderDrawRect(renderer, &bg);
        SDL_Rect borderInner = {x + 1, y + 1, boxSize - 2, boxSize - 2};
        SDL_RenderDrawRect(renderer, &borderInner);
        
       
        if (cooldown > 0.0f) {
            SDL_Color cooldownOverlay = {20, 20, 20, 180};
            setColor(cooldownOverlay);
            SDL_Rect overlay = {x, y, boxSize, boxSize};
            SDL_RenderFillRect(renderer, &overlay);
            
            float progress = cooldown / (cooldown > 50.0f ? 80.0f : cooldown > 10.0f ? 18.0f : cooldown > 8.0f ? 12.0f : 4.0f);
            if (progress > 1.0f) progress = 1.0f;
            int barHeight = static_cast<int>((boxSize - 4) * progress);
            
            SDL_Color progressColor = {200, 50, 50, 180};
            setColor(progressColor);
            SDL_Rect progressBar = {x + 2, y + boxSize - 2 - barHeight, boxSize - 4, barHeight};
            SDL_RenderFillRect(renderer, &progressBar);
            

            setColor({255, 255, 255, 255});
            int seconds = static_cast<int>(std::ceil(cooldown));
            

            int digitX = x + boxSize/2 - 8;
            int digitY = y + boxSize/2 - 10;
            
            if (seconds >= 10) {

                int tens = seconds / 10;
                drawDigit(digitX - 8, digitY, tens);
                drawDigit(digitX + 8, digitY, seconds % 10);
            } else {
         
                drawDigit(digitX, digitY, seconds);
            }
        } else {
        
            SDL_Color readyGlow = {80, 255, 80, 100};
            setColor(readyGlow);
            SDL_Rect glow = {x + 2, y + 2, boxSize - 4, boxSize - 4};
            SDL_RenderFillRect(renderer, &glow);
        }
        
 
        setColor(keyColor);
  
        int labelX = x + boxSize/2 - 6;
        int labelY = y + boxSize - 15;
        
        if (key[0] == 'Q') {
   
            SDL_Rect q[] = {
                {labelX, labelY, 12, 2}, {labelX, labelY, 2, 12}, 
                {labelX + 10, labelY, 2, 12}, {labelX, labelY + 10, 12, 2},
                {labelX + 8, labelY + 9, 5, 2}
            };
            for (auto& r : q) SDL_RenderFillRect(renderer, &r);
        } else if (key[0] == 'W') {
         
            SDL_Rect w[] = {
                {labelX, labelY, 2, 12}, {labelX + 5, labelY + 5, 2, 7},
                {labelX + 10, labelY, 2, 12}
            };
            for (auto& r : w) SDL_RenderFillRect(renderer, &r);
        } else if (key[0] == 'E') {
       
            SDL_Rect e[] = {
                {labelX, labelY, 2, 12}, {labelX, labelY, 12, 2},
                {labelX, labelY + 5, 10, 2}, {labelX, labelY + 10, 12, 2}
            };
            for (auto& r : e) SDL_RenderFillRect(renderer, &r);
        } else if (key[0] == 'R') {
            
            SDL_Rect rr[] = {
                {labelX, labelY, 2, 12},          
                {labelX, labelY, 12, 2},          
                {labelX + 10, labelY, 2, 6},      
                {labelX, labelY + 5, 12, 2},     
                {labelX + 5, labelY + 6, 2, 2},  
                {labelX + 7, labelY + 8, 2, 2},   
                {labelX + 9, labelY + 10, 2, 2}   
            };
            for (auto& r : rr) SDL_RenderFillRect(renderer, &r);
        }
        
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    };
    

    SDL_Color qColor = {255, 200, 100, 255}; 
    SDL_Color wColor = {100, 200, 255, 255};  
    SDL_Color eColor = {255, 100, 150, 255};  
    SDL_Color rColor = {200, 100, 255, 255};  
    
    renderCooldownBox(startX, startY, "Q", qCooldown, qColor);
    renderCooldownBox(startX + boxSize + spacing, startY, "W", wCooldown, wColor);
    renderCooldownBox(startX + (boxSize + spacing) * 2, startY, "E", eCooldown, eColor);
    renderCooldownBox(startX + (boxSize + spacing) * 3, startY, "R", rCooldown, rColor);
}

void Renderer::drawDigit(int x, int y, int digit) {
   
    const int w = 12;
    const int h = 16;
    
    SDL_Rect segments[7] = {
        {x, y, w, 2},           
        {x + w - 2, y, 2, h/2}, 
        {x + w - 2, y + h/2, 2, h/2}, 
        {x, y + h - 2, w, 2},   
        {x, y + h/2, 2, h/2},   
        {x, y, 2, h/2},        
        {x, y + h/2 - 1, w, 2}  
    };
    
    bool patterns[10][7] = {
        {1,1,1,1,1,1,0},
        {0,1,1,0,0,0,0}, 
        {1,1,0,1,1,0,1}, 
        {1,1,1,1,0,0,1}, 
        {0,1,1,0,0,1,1}, 
        {1,0,1,1,0,1,1}, 
        {1,0,1,1,1,1,1}, 
        {1,1,1,0,0,0,0}, 
        {1,1,1,1,1,1,1}, 
        {1,1,1,1,0,1,1}  
    };
    
    if (digit >= 0 && digit <= 9) {
        for (int i = 0; i < 7; i++) {
            if (patterns[digit][i]) {
                SDL_RenderFillRect(renderer, &segments[i]);
            }
        }
    }
}
