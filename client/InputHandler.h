#ifndef BF090E70_F2B6_4557_8427_BCBF57C726FD
#define BF090E70_F2B6_4557_8427_BCBF57C726FD
#ifndef INPUTHANDLER_H
#define INPUTHANDLER_H

#include <SDL2/SDL.h>
#include <utility>

class InputHandler {
private:
    bool keyStates[SDL_NUM_SCANCODES];
    int mouseX, mouseY;
    bool mouseButtons[3];
    bool quit;

public:
    InputHandler();
    
    void update();
    void handleEvent(const SDL_Event& event);
    
    std::pair<float, float> getMovementVector() const;
    bool isShooting() const;
    std::pair<float, float> getShootDirection(float playerX, float playerY) const;
    bool shouldQuit() const { return quit; }
    
    void reset();
};

#endif // INPUTHANDLER_H


#endif /* BF090E70_F2B6_4557_8427_BCBF57C726FD */
