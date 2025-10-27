#ifndef BBADE07D_82F9_4776_BE73_9A9D7B72D779
#define BBADE07D_82F9_4776_BE73_9A9D7B72D779
#ifndef RENDERER_H
#define RENDERER_H

#include <SDL2/SDL.h>
#include <memory>
#include <unordered_map>
#include "../shared/Entities.h"
#include "../shared/Config.h"

class Renderer {
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    int width;
    int height;
    
    float cameraX;
    float cameraY;
    float cameraScale;
    
    SDL_Color bgColor;
    SDL_Color grassColor1;       
    SDL_Color grassColor2;        
    SDL_Color grassColor3;        
    SDL_Color samuraiColor;       
    SDL_Color samuraiSwordColor;  
    SDL_Color windColor;         
    SDL_Color otherPlayerColor;
    SDL_Color gridColor;
    SDL_Color textColor;
    SDL_Color healthBarGreen;
    SDL_Color healthBarRed;

public:
    Renderer(int w = Config::WORLD_WIDTH, int h = Config::WORLD_HEIGHT);
    ~Renderer();

    bool initialize();
    void clear();
    void present();
    
    // Camera methods
    void updateCamera(float playerX, float playerY);
    void worldToScreen(float worldX, float worldY, int& screenX, int& screenY);
    void screenToWorld(int screenX, int screenY, float& worldX, float& worldY);
    
    void renderGrass();
    void renderDecorations();
    void renderGrid();
    void renderPlayer(const Player& player, bool isLocal = false);
    void renderDummy(const Dummy& dummy);
    void renderProjectile(const Projectile& projectile);
    void renderWindWall(const Player& player);
  
    void renderSwordSwing(float originX, float originY, float rotationRad,
                          float arcDegrees, float range);
    void renderDashBurst(float originX, float originY, float rotationRad, float length);
    void renderRTornado(float x, float y);
    void renderUI(uint32_t playerId, int playerCount, int fps);
    
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    
private:
    void setColor(const SDL_Color& color);
    void renderCircle(int centerX, int centerY, int radius);
    void renderText(const char* text, int x, int y, int size = 20);
    
    // Decoration rendering helpers
    void renderCabin(int worldX, int worldY);
    void renderSpruce(int worldX, int worldY);
    void renderCampfire(int worldX, int worldY);
    bool shouldSpawnDecoration(int worldX, int worldY, int decorationType);
};

#endif // RENDERER_H


#endif /* BBADE07D_82F9_4776_BE73_9A9D7B72D779 */
