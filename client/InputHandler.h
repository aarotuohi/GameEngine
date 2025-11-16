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
    
    // Right-click movement target
    bool hasMovementTarget;
    float targetX, targetY;
    
    // Ability key presses 
    bool qPressed, wPressed, ePressed, rPressed;

public:
    InputHandler();
    
    void update();
    void handleEvent(const SDL_Event& event);
    
    // New movement system
    bool hasTarget() const { return hasMovementTarget; }
    std::pair<float, float> getTarget() const { return {targetX, targetY}; }
    void clearTarget() { hasMovementTarget = false; }
    
    // Ability detection
    bool isQPressed() const { return qPressed; }
    bool isWPressed() const { return wPressed; }
    bool isEPressed() const { return ePressed; }
    bool isRPressed() const { return rPressed; }
    void clearAbilityInputs();
    
    bool isKeyPressed(SDL_Keycode key) const;
    
   
    bool isLeftMousePressed() const { return mouseButtons[0]; }
    int getMouseX() const { return mouseX; }
    int getMouseY() const { return mouseY; }
    
    // Legacy methods

    
    std::pair<float, float> getMovementVector() const;
 
    std::pair<float, float> getShootDirection(float playerX, float playerY) const;
    bool shouldQuit() const { return quit; }
    void requestQuit() { quit = true; }
    
    void reset();
};

#endif // INPUTHANDLER_H


#endif /* BF090E70_F2B6_4557_8427_BCBF57C726FD */
