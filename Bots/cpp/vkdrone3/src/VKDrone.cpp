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

VKDrone::VKDrone() : rotationPID(KP_ROT, KI_ROT, KD_ROT, -1.0, 1.0, 0.05),
                     movementPID(KP_MOVE, KI_MOVE, KD_MOVE, 0.0,
                                 MAIN_THRUST_POWER, 0.05){
    // Use current time as seed for rand.
    srand(time(0));
    nextCharge = 1;
}

VKDrone::~VKDrone() {
}

void VKDrone::Process() {
    updateFacingAngle();

    // Fill threats list with rocks and lasers near the ship.
    updateNearThreats(NEAR_DIST);

    updateNextCharge();

    GameObject *target = closestObject<Ship>(gameState->ships);
    if (target == NULL) {
        // Do nothing when there is no enemies left.
        // TODO: it should avoid to die in order to not lose a score point.
        return;
    }

    // Rotate to face the target.
    double laserSpeed = nextCharge * LASER_BASE_SPEED;
    double angleToTarget = angleTo(futurePosition(target, laserSpeed));
    Point2D p = {15, 0};
    goTo(p, angleToTarget);

    // Shoot only when is aiming at right direction (angle < 1 degree).
    if (myShip->charge >= nextCharge && angleToTarget <= 0.01745) {
        //shoot = nextCharge;
        nextCharge = -1;
    } else {
        shoot = 0;
    }

    // velAngMaxThrust = 534.38
    // velAngMaxFree   = 500.00
    // timeStep        = 0.0500 (seconds)
    // acelAngMax      = +-2062.65
    // laserSpeed1     = 25 (m/s)
    // laserSpeed2     = 50 (m/s)
    // laserSpeed3     = 75 (m/s)
    // chargeTime      = 0.5 (s per slot)
}

double VKDrone::angleTo(const GameObject *obj) {
    return angleTo(obj->posx, obj->posy);
}

double VKDrone::angleTo(Point2D point) {
    return angleTo(point.x, point.y);
}

double VKDrone::angleTo(double x, double y) {

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

    return angleToTarget;
}

void VKDrone::updateFacingAngle() {
    // Angle from NORTH to "ang" attribute restricted to [-2PI, 2PI].
    facingAngle = fmod(myShip->ang, 360) * (PI / 180.0);

    // Ajust facingAngle to [-PI, PI] so that:
    // North = 0; West = PI/2; South = +/-PI; East = -PI/2.
    if (facingAngle > PI) {
        facingAngle -= 2 * PI;
    } else if (facingAngle < -PI) {
        facingAngle += 2 * PI;
    }
}

Point2D VKDrone::futurePosition(const GameObject *obj, double laserSpeed) {
    // Distance to obj.
    double dist = sqrt(pow(obj->posx - myShip->posx, 2) +
                       pow(obj->posy - myShip->posy, 2));

    // Time needed to a laser beam travel all dist.
    double dt = dist / laserSpeed;

    Point2D nextPos = {
        obj->posx + obj->velx * dt,
        obj->posy + obj->vely * dt
    };
    return nextPos;
}

void VKDrone::updateNearThreats(double nearDist) {
    // Bound box of near objects.
    Point2D upperLeft   = {myShip->posx - nearDist, myShip->posy - nearDist};
    Point2D bottomRight = {myShip->posx + nearDist, myShip->posy + nearDist};

    nearThreats.clear();

    // Lasers.
	for (map<int, Laser*>::iterator itr = gameState->lasers.begin();
         itr != gameState->lasers.end(); itr++) {

        Laser *obj = itr->second;
        if (isInsideBox(obj, upperLeft, bottomRight)) {
            nearThreats.push_back(obj);
        }
    }

    // Rocks.
	for (map<int, Rock*>::iterator itr = gameState->rocks.begin();
         itr != gameState->rocks.end(); itr++) {

        Rock *obj = itr->second;
        if (isInsideBox(obj, upperLeft, bottomRight)) {
            nearThreats.push_back(obj);
        }
    }

    gameState->Log("box size: " + to_string(nearThreats.size()));
}

void VKDrone::updateNextCharge() {
    if (nextCharge == -1) {
        // Get pseudo-random shot power;
        nextCharge = 1 + static_cast<int>(rand() % 2);
    }

    // If has charged a faster shot already, use it.
    if (myShip->charge >= nextCharge) {
        nextCharge = static_cast<int>(myShip->charge);
    }
}

void VKDrone::goTo(Point2D destiny, double angle) {
    double deltaX = destiny.x - myShip->posx;
    double deltaY = destiny.y - myShip->posy;
    double distToDestiny = sqrt(pow(deltaX, 2) + pow(deltaY, 2));
    double pidDist = movementPID.compute(0.0, -distToDestiny);

    double Fx = pidDist * (deltaX / distToDestiny);
    double Fy = pidDist * (deltaY / distToDestiny);
    double Fr = angle;

    double sinA = sin(facingAngle);
    double cosA = cos(facingAngle);

    double M = MAIN_THRUST_POWER;
    double S = SIDE_THRUST_POWER;
    double T = SIDE_THRUST_TORQUE;

    double m; // Main thrust.
    double f; // Front side thrust.
    double b; // Back side thrust.

    if (fabs(sinA) <= ZERO) {
        m = Fy / (M * cosA);
        f = 0.5 * ((Fx / (S * cosA)) - (Fr / T));
        b = 0.5 * ((Fx / (S * cosA)) + (Fr / T));
    } else if (fabs(cosA) <= ZERO) {
        m = -Fx / (M * sinA);
        f = 0.5 * ((Fy / (S * sinA)) - (Fr / T));
        b = 0.5 * ((Fy / (S * sinA)) + (Fr / T));
    } else {
        m = (Fx - (cosA / sinA) * Fy) / (M * (-sinA - (cosA * cosA / sinA)));
        b = 0.5 * (((Fy - m * M * cosA) / (S * sinA)) + Fr / T);
        f = b - (Fr / T);
    }

    double rotationBoost = rotationPID.compute(0.0, angle);
    f += rotationBoost;
    b -= rotationBoost;

    thrust = m;
    sideThrustFront = f;
    sideThrustBack = b;
}

/*
Point2D VKDrone::evadePosition() {

}
*/


/*******************************************************************************
 * "Hidden" auxiliary functions.
 * It shuld be refactored.
 ******************************************************************************/

bool isInsideBox(GameObject *obj, Point2D upperLeft, Point2D bottomRight) {
    return obj->posx >= upperLeft.x && obj->posx <= bottomRight.x &&
           obj->posy >= upperLeft.y && obj->posy <= bottomRight.y;
}
