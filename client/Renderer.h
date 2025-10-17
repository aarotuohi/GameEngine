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
    
    // Colors
    SDL_Color bgColor;
    SDL_Color playerColor;
    SDL_Color taggedColor;
    SDL_Color otherPlayerColor;
    SDL_Color gridColor;
    SDL_Color textColor;

public:
    Renderer(int w = Config::WORLD_WIDTH, int h = Config::WORLD_HEIGHT);
    ~Renderer();

    bool initialize();
    void clear();
    void present();
    
    void renderGrid();
    void renderPlayer(const Player& player, bool isLocal = false);
    void renderUI(uint32_t playerId, int playerCount, int fps);
    
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    
private:
    void setColor(const SDL_Color& color);
    void renderCircle(int centerX, int centerY, int radius);
    void renderText(const char* text, int x, int y, int size = 20);
};

#endif // RENDERER_H


#endif /* BBADE07D_82F9_4776_BE73_9A9D7B72D779 */
