#include "BotBase.h"

BotBase::BotBase() {
    gameState = NULL;
    myShip = NULL;
    thrust = 0.0f;
    sideThrustFront = 0.0f;
    sideThrustBack = 0.0f;
    shoot = 0;
}

BotBase::~BotBase() {
    // Destructor.
}

void BotBase::Update() {
    if(gameState != NULL) {
        if(myShip != NULL) {
            Process();
        }

        gameState->SetThrust(thrust);
        gameState->SetSideThrustFront(sideThrustFront);
        gameState->SetSideThrustBack(sideThrustBack);
        gameState->SetShoot(shoot);
    }
}

void BotBase::Process() {
    // Should be implemented by child class.
}
