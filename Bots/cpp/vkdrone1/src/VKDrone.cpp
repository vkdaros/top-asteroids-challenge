#include <cmath>
#include <string>

#include "VKDrone.h"

using std::map;

using std::pow;
using std::sqrt;
using std::sin;
using std::cos;
using std::acos;

VKDrone::VKDrone() {
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
    */

    // Rotate to face the target.
    aimAt(target);
    thrust = 0.01f;

    /*
    gameState->Log("ang: "  + to_string(target->ang) +
                   "; va: " + to_string(target->velAng));
    */
}

Ship* VKDrone::closestShip(map<int, Ship*> &ships) {
    // Squared distance to closest enemy ship.
    float shortestDist2 = -1;

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
        float dist2 = pow(ship->posx - myShip->posx, 2) +
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

void VKDrone::aimAt(float x, float y) {
    // Angle from NORTH to "ang" attribute restricted to [-2PI, 2PI].
    float facingAngle = fmod(myShip->ang, 360) * (PI / 180.0f);

    // Ajust facingAngle to [-PI, PI]. West -> negative, East -> positive.
    if (facingAngle > PI) {
        facingAngle -= 2 * PI;
    } else if (facingAngle < -PI) {
        facingAngle += 2 * PI;
    }

    // Unitary vector of facing direction from NORTH.
    float facingX = -1 * sin(facingAngle);
    float facingY = cos(facingAngle);

    // Vector to target with respect to myShip.
    float targetX = x - myShip->posx;
    float targetY = y - myShip->posy;

    // Find the angle alpha between the vectors using "dot (or inner) product".
    // Note: alpha is in [0,PI] because of acos.
    float alpha = acos((facingX * targetX + facingY * targetY) /
                       (
                           sqrt(facingX * facingX + facingY * facingY) *
                           sqrt(targetX * targetX + targetY * targetY)
                       ));

    // Cross product sign tells if target is at right (negative alpha).
    if (facingX * targetY - facingY * targetX < 0) {
        alpha *= -1.0f;
    }

    gameState->Log("facing: " + to_string(facingAngle) + " rad: " + to_string(alpha));

    sideThrustFront = alpha * -0.1f;
    sideThrustBack = -sideThrustFront;
}
