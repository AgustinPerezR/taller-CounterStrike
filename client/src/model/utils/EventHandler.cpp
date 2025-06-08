#include "../../../../client/include/model/utils/EventHandler.h"

#include <SDL2/SDL.h>

#include "../../../../client/dtos/AimInfo.h"
#include "../../../../common/utils/Vec2D.h"

EventHandler::EventHandler(Client* client, World& world): client(client), world(world) {}

void EventHandler::handleEvents(bool& gameIsRunning) const {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            gameIsRunning = false;
            return;
        }
    }

    handleKeyboardEvents(gameIsRunning);
    handleMouseEvents(gameIsRunning);
}

void EventHandler::handleKeyboardEvents(bool& gameIsRunning) const {
    const Uint8* state = SDL_GetKeyboardState(NULL);

    Vec2D direction(0, 0);

    if (state[SDL_SCANCODE_A]) {
        direction.setX(-1);
    } else if (state[SDL_SCANCODE_D]) {
        direction.setX(1);
    }

    if (state[SDL_SCANCODE_W]) {
        direction.setY(-1);
    } else if (state[SDL_SCANCODE_S]) {
        direction.setY(1);
    }

    if ((direction.getX() != 0) || (direction.getY() != 0)) {
        client->move(direction);
    }


    if (state[SDL_SCANCODE_ESCAPE])
        gameIsRunning = false;
}

void EventHandler::handleMouseEvents(bool gameIsRunning) const {
    if (!gameIsRunning) {
        return;
    }

    int mouseX, mouseY;
    Uint32 mouseButtons = SDL_GetMouseState(&mouseX, &mouseY);

    AimInfo aimInfo = world.getPlayerAimInfo(mouseX, mouseY);

    if (mouseButtons & SDL_BUTTON(SDL_BUTTON_LEFT)) {
        client->shoot(aimInfo);
    } else {
        client->rotate(aimInfo.angle);
    }
}
