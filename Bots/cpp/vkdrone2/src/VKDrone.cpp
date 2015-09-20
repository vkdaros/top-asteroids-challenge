#include <cmath>
#include <ctime>
#include <cstdlib>
#include <string>

#include "VKDrone.h"

using std::map;
using std::time;
using std::rand;
using std::srand;

using std::pow;
using std::sqrt;
using std::sin;
using std::cos;
using std::acos;

VKDrone::VKDrone() : rotationPID(KP_ROT, KI_ROT, KD_ROT, -1.0, 1.0, 0.05) {
    // Use current time as seed for rand.
    srand(time(0));
    charge = 1;
}

VKDrone::~VKDrone() {
}

void VKDrone::Process() {
    GameObject *target = closestObject<Ship>(gameState->ships);
    if (target == NULL) {
        // Do nothing when there is no enemies left.
        // TODO: it should avoid to die in order to not lose a score point.
        return;
    }

    if (charge == -1) {
        // Get pseudo-random shot power;
        charge = 1 + static_cast<int>(rand() % 2);
        gameState->Log("charge: " + to_string(charge));
    }

    // Rotate to face the target.
    double laserSpeed = charge * LASER_BASE_SPEED;
    aimAt(futurePosition(target, laserSpeed));

    if (myShip->charge >= charge) {
        shoot = charge;
        charge = -1;
    } else {
        shoot = 0;
    }

    //thrust = 0.1;

    // velAngMaxThrust = 534.38
    // velAngMaxFree   = 500.00
    // timeStep        = 0.0500 (seconds)
    // acelAngMax      = +-2062.65
    // laserSpeed1     = 25 (m/s)
    // laserSpeed2     = 50 (m/s)
    // laserSpeed3     = 75 (m/s)
    // chargeTime      = 0.5 (s per slot)
}

void VKDrone::aimAt(const GameObject *obj) {
    return aimAt(obj->posx, obj->posy);
}

void VKDrone::aimAt(Point2D point) {
    return aimAt(point.x, point.y);
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

Point2D VKDrone::futurePosition(const GameObject *obj, double laserSpeed) {
    // Distance to obj.
    double dist = sqrt(pow(obj->posx - myShip->posx, 2) +
                       pow(obj->posy - myShip->posy, 2));

    // Time needed to a laser beam travel all dist.
    double dt = dist / laserSpeed;

    Point2D nextPos;
    nextPos.x = obj->posx + obj->velx * dt;
    nextPos.y = obj->posy + obj->vely * dt;

    return nextPos;
}
