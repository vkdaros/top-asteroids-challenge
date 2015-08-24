#include <cmath>
#include <string>

#include "VKDrone.h"

VKDrone::VKDrone() {
}

VKDrone::~VKDrone() {
}

void VKDrone::Process() {
    int target = -2;
    target = closestShip(gameState->ships);
    gameState->Log("uid: " + std::to_string(target) +
                   " x: "  + std::to_string(gameState->ships[target]->posx) +
                   " y: "  + std::to_string(gameState->ships[target]->posy));

    //sideThrustFront = 1.0f;
    //sideThrustBack = 1.0f;
}

int VKDrone::closestShip(std::map<int, Ship*> &ships) {
    // Squared distance to closest enemy ship.
    float shortestDist2 = -1;

    // ID of closest enemy ship.
    int uid = -1;

	for(std::map<int, Ship*>::iterator itr = ships.begin();
        itr != ships.end(); itr++) {

        Ship *ship = itr->second;

        // Ignore myShip.
        if (ship->uid == myShip->uid) {
            continue;
        }

        // Squared distance to enemy ship.
        float dist2 = std::pow(ship->posx - myShip->posx, 2) +
                      std::pow(ship->posy - myShip->posy, 2);

        // Update closest enemy information.
        if (dist2 < shortestDist2 || shortestDist2 <= 0) {
            shortestDist2 = dist2;
            uid = ship->uid;
        }
    }

    return uid;
}
