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
    firstRun = true;
    evading = false;
}

VKDrone::~VKDrone() {
}

void VKDrone::Process() {
    if (firstRun) {
        firstRun = false;
        destiny.x = myShip->posx;
        destiny.y = myShip->posy;
    }
    updateFacingAngle();

    // Fill threats list with rocks and lasers near the ship.
    updateNearThreats(NEAR_DIST);

    updateNextCharge();

    double angleToTarget;
    GameObject *target = closestObject<Ship>(gameState->ships);
    if (target != NULL) {
        double laserSpeed = nextCharge * LASER_BASE_SPEED;
        angleToTarget = angleTo(futurePosition(target, laserSpeed));
    } else {
        angleToTarget = angleTo(0.0, 0.0);
    }

    destiny = evadePosition();
    goTo(destiny, angleToTarget);

    // If all enemies has died, keep shooting.
    shoot = 0;
    if (target == NULL) {
        shoot = 1;
        nextCharge = 1;
    } else if (myShip->charge >= nextCharge && angleToTarget <= 0.01745) {
        // Shoot only when aiming at right direction (angle < 1 degree) and with
        // correct laser power charged.
        shoot = nextCharge;
        nextCharge = -1;
    }
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
}

void VKDrone::updateNextCharge() {
    if (nextCharge == -1) {
        // Get pseudo-random shot power;
        nextCharge = 1 + static_cast<int>(rand() % 3);
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

    double Fx = 0.0;
    double Fy = 0.0;
    if (fabs(distToDestiny) > ZERO) {
        double pidDist = movementPID.compute(0.0, -distToDestiny);
        Fx = pidDist * (deltaX / distToDestiny);
        Fy = pidDist * (deltaY / distToDestiny);
    }
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

Point2D VKDrone::evadePosition() {
    vector<Point2D> threatsDirections;
    double dt = gameState->timeStep;

    Point2D myNextPos = {
        myShip->posx + myShip->velx * dt,
        myShip->posy + myShip->vely * dt
    };

    for (unsigned int i = 0; i < nearThreats.size(); i++) {
        GameObject *obj = nearThreats[i];

        Point2D objPos = {obj->posx, obj->posy};
        Point2D objNextPos = {
            objPos.x + obj->velx * dt,
            objPos.y + obj->vely * dt
        };

        double dist = distPointToLine(myNextPos, objPos, objNextPos, gameState);
        double minDist = myShip->radius + obj->radius;
        if (fabs(dist) <= ZERO) {
            Point2D v = {
                objNextPos.y - myNextPos.y,
                objNextPos.x - myNextPos.x
            };
            v.x += myNextPos.x;
            v.y += myNextPos.y;
            threatsDirections.push_back(v);
        } else if (dist <= minDist) {
            double u = collisionDistance(myNextPos, objPos, objNextPos);
            Point2D collisionPoint = {
                objPos.x + u * (objNextPos.x - objPos.x),
                objPos.y + u * (objNextPos.y - objPos.y)
            };
            threatsDirections.push_back(collisionPoint);
        }
    }

    if (threatsDirections.size() == 0) {
        if (evading) {
            destiny.x = myShip->posx;
            destiny.y = myShip->posy;
        }
        evading = false;
    } else {
        evading = true;
    }

    Point2D safeDirection = {destiny.x, destiny.y};
    for (unsigned int i = 0; i < threatsDirections.size(); i++) {
        safeDirection.x += myNextPos.x - threatsDirections[i].x;
        safeDirection.y += myNextPos.y - threatsDirections[i].y;
    }

    return safeDirection;
}


/*******************************************************************************
 * "Hidden" auxiliary functions.
 * It shuld be refactored.
 ******************************************************************************/

bool isInsideBox(GameObject *obj, Point2D upperLeft, Point2D bottomRight) {
    return obj->posx >= upperLeft.x && obj->posx <= bottomRight.x &&
           obj->posy >= upperLeft.y && obj->posy <= bottomRight.y;
}

double distPointToLine(Point2D p, Point2D from, Point2D to, GameState *g) {
    double numerator = fabs((to.x - from.x)*(from.y - p.y) -
                            (from.x - p.x)*(to.y - from.y));

    double denominator = sqrt(pow(to.x - from.x, 2) + pow(to.y - from.y, 2));

    return numerator / denominator;
}

double collisionDistance(Point2D p, Point2D from, Point2D to) {
    double numerator = (p.x - from.x)*(to.x - from.x) +
                       (p.y - from.y)*(to.y - from.y);

    double denominator = pow(to.x - from.x, 2) + pow(to.y - from.y, 2);

    return numerator / denominator;
}
