#include <iostream>

#include "bot_interface.h"
#include "VKDrone.h"

int main(int argc, char *argv[]) {
    GameState *gameState = new GameState();

    //Change to your bot's class name
    VKDrone *bot = new VKDrone();

    bot->gameState = gameState;
    bot->myShip = gameState->myShip;

    gameState->Log("Loaded");

    while (bot->myShip != NULL) {
        gameState->Update();
        bot->Update();
        gameState->WriteData();
    }

    delete gameState;
    delete bot;

    return 0;
}
