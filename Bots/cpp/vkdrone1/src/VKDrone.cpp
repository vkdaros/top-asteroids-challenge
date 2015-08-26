#include <cmath>
#include <string>

#include "VKDrone.h"

using std::map;

using std::pow;
using std::sqrt;
using std::sin;
using std::cos;
using std::acos;

VKDrone::VKDrone() : rotationPID(KP_ROT, KI_ROT, KD_ROT, -1.0, 1.0, 0.05) {
}

VKDrone::~VKDrone() {
}

void VKDrone::Process() {
    Ship *target = closestShip(gameState->ships);
    if (target == NULL) {
        // Do nothing when there is no enemies left.
        return;
    }
    /*
    gameState->Log("x: "  + to_string(target->posx) +
                   " y: " + to_string(target->posy));
    gameState->Log("tic: "  + to_string(gameState->tick) +
                   "; d: " + to_string(gameState->timeStep));
    gameState->Log("ang: "  + to_string(target->ang) +
                   "; va: " + to_string(target->velAng));
    */

    // Rotate to face the target.
    aimAt(target);
    thrust = 0.1;

    if (myShip->charge >= 3 && fabs(myShip->velAng) < 5) {
        shoot = 3;
    } else {
        shoot = 0;
    }

    // velAngMaxThrust = 534.38
    // velAngMaxFree   = 500.00
    // timeStep        = 0.0500 (seconds)
    // acelAngMax      = +-2062.65
}

Ship* VKDrone::closestShip(map<int, Ship*> &ships) {
    // Squared distance to closest enemy ship.
    double shortestDist2 = -1;

    // ID of closest enemy ship.
    // Initial value just to check later if any enemy was found.
    int uid = myShip->uid;

	for(map<int, Ship*>::iterator itr = ships.begin();
        itr != ships.end(); itr++) {

        Ship *ship = itr->second;

        // Ignore myShip.
        if (ship->uid == myShip->uid) {
            continue;
        }

        // Squared distance to enemy ship.
        double dist2 = pow(ship->posx - myShip->posx, 2) +
                      pow(ship->posy - myShip->posy, 2);

        // Update closest enemy information.
        if (dist2 < shortestDist2 || shortestDist2 <= 0) {
            shortestDist2 = dist2;
            uid = ship->uid;
        }
    }

    if (uid == myShip->uid) {
        // No enemy found.
        return NULL;
    }
    return gameState->ships[uid];
}

void VKDrone::aimAt(const GameObject *obj) {
    return aimAt(obj->posx, obj->posy);
}

void VKDrone::aimAt(double x, double y) {
    // Angle from NORTH to "ang" attribute restricted to [-2PI, 2PI].
    double facingAngle = fmod(myShip->ang, 360) * (PI / 180.0);

    // Ajust facingAngle to [-PI, PI]. West -> negative, East -> positive.
    if (facingAngle > PI) {
        facingAngle -= 2 * PI;
    } else if (facingAngle < -PI) {
        facingAngle += 2 * PI;
    }

    // Unitary vector of facing direction from NORTH.
    double facingX = -1 * sin(facingAngle);
    double facingY = cos(facingAngle);

    // Vector to target with respect to myShip.
    double targetX = x - myShip->posx;
    double targetY = y - myShip->posy;

    // Find the angle [0,PI] between the vectors using "dot (or inner) product".
    double dotProduct = facingX * targetX + facingY * targetY;
    double norm = sqrt(facingX * facingX + facingY * facingY) *
                  sqrt(targetX * targetX + targetY * targetY);
    double result = max(min(dotProduct / norm, 1.0), -1.0);
    double angleToTarget = acos(result);

    // Cross product sign tells if target is at right (negative angleToTarget).
    if (facingX * targetY - facingY * targetX < 0) {
        angleToTarget *= -1.0;
    }

    sideThrustFront = rotationPID.compute(0.0, angleToTarget);
    sideThrustBack = -sideThrustFront;
}
