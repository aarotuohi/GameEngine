#include "InputHandler.h"
#include <cmath>
#include <cstring>

InputHandler::InputHandler() 
    : mouseX(0), mouseY(0), quit(false), 
      hasMovementTarget(false), targetX(0), targetY(0),
      qPressed(false), wPressed(false), ePressed(false), rPressed(false) {
    std::memset(keyStates, 0, sizeof(keyStates));
    std::memset(mouseButtons, 0, sizeof(mouseButtons));
}

void InputHandler::update() {
    // Get current keyboard state
    const Uint8* currentKeyStates = SDL_GetKeyboardState(nullptr);
    std::memcpy(keyStates, currentKeyStates, sizeof(keyStates));
    
    // Get mouse state
    Uint32 mouseState = SDL_GetMouseState(&mouseX, &mouseY);
    mouseButtons[0] = (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;
    mouseButtons[1] = (mouseState & SDL_BUTTON(SDL_BUTTON_MIDDLE)) != 0;
    mouseButtons[2] = (mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0;
}

void InputHandler::handleEvent(const SDL_Event& event) {
    if (event.type == SDL_QUIT) {
        quit = true;
    } else if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_ESCAPE) {
            quit = true;
        }
        // Ability keys - detect single press
        else if (event.key.keysym.sym == SDLK_q) {
            qPressed = true;
        }
        else if (event.key.keysym.sym == SDLK_w) {
            wPressed = true;
        }
        else if (event.key.keysym.sym == SDLK_e) {
            ePressed = true;
        }
        else if (event.key.keysym.sym == SDLK_r) {
            rPressed = true;
        }
    } else if (event.type == SDL_MOUSEBUTTONDOWN) {
        if (event.button.button == SDL_BUTTON_RIGHT) {
            // Right-click sets movement target
            hasMovementTarget = true;
            targetX = static_cast<float>(event.button.x);
            targetY = static_cast<float>(event.button.y);
        }
    }
}

std::pair<float, float> InputHandler::getMovementVector() const {
    float dx = 0.0f, dy = 0.0f;
    
    // WASD or Arrow keys
    if (keyStates[SDL_SCANCODE_W] || keyStates[SDL_SCANCODE_UP]) {
        dy -= 1.0f;
    }
    if (keyStates[SDL_SCANCODE_S] || keyStates[SDL_SCANCODE_DOWN]) {
        dy += 1.0f;
    }
    if (keyStates[SDL_SCANCODE_A] || keyStates[SDL_SCANCODE_LEFT]) {
        dx -= 1.0f;
    }
    if (keyStates[SDL_SCANCODE_D] || keyStates[SDL_SCANCODE_RIGHT]) {
        dx += 1.0f;
    }
    
    // Normalize diagonal movement
    if (dx != 0.0f && dy != 0.0f) {
        float length = std::sqrt(dx * dx + dy * dy);
        dx /= length;
        dy /= length;
    }
    
    return {dx, dy};
}



std::pair<float, float> InputHandler::getShootDirection(float playerX, float playerY) const {
    float dx = static_cast<float>(mouseX) - playerX;
    float dy = static_cast<float>(mouseY) - playerY;
    
    float length = std::sqrt(dx * dx + dy * dy);
    if (length > 0.0f) {
        return {dx / length, dy / length};
    }
    return {0.0f, 0.0f};
}

void InputHandler::clearAbilityInputs() {
    qPressed = false;
    wPressed = false;
    ePressed = false;
    rPressed = false;
}

void InputHandler::reset() {
    quit = false;
    hasMovementTarget = false;
    clearAbilityInputs();
}
