#include <cmath>
#include <ctime>
#include <cstdlib>
#include <string>

#include "VKDrone.h"

using std::map;
using std::time;
using std::rand;
using std::srand;

using std::min;
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
    holdFire = false;
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

    // Find the closest enemy.
    GameObject *target = closestObject<Ship>(gameState->ships);
    if (target == NULL) {
        // If there is no enemy, find closest rock.
        target = closestObject<Rock>(gameState->rocks);
        nextCharge = 1;
    }
    double angleToTarget;
    if (target == NULL) {
        // If there is no rock, aim to the center of arena.
        angleToTarget = angleTo(0.0, 0.0);
    } else if (evading) {
        angleToTarget = angleTo(target);
    } else {
        double laserSpeed = nextCharge * LASER_BASE_SPEED;
        angleToTarget = angleTo(futurePosition(target, laserSpeed));
    }

    destiny = evadePosition();
    goTo(destiny, angleToTarget);

    // If all enemies has died, keep shooting.
    shoot = 0;
    if (target == NULL || nearThreats.size() >= 4) {
        shoot = 1;
        nextCharge = 1;
    } else if (!holdFire && myShip->charge >= nextCharge &&
               fabs(angleToTarget) <= 0.017) {
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

Point2D VKDrone::futurePosition(const GameObject *obj, double ls) {
    double vx = obj->velx;
    double vy = obj->vely;
    Point2D T = {obj->posx, obj->posy};
    Point2D N = {myShip->posx, myShip->posy};

    double a = vx*vx + vy*vy - ls*ls;
    double b = 2.0 * ((T.x - N.x)*vx + (T.y - N.y)*vy);
    double c = (T.x - N.x)*(T.x - N.x) + (T.y - N.y)*(T.y - N.y);

    double dt = 0;

    if (fabs(a) <= ZERO) {
        if (fabs(b) <= ZERO) {
            dt = c;
        } else {
            dt = -c / b;
        }
        if (fabs(dt) <= ZERO) {
            holdFire = true;
            // Fake prediction.
            dt = gameState->timeStep;
        }
    } else {
        double D = b*b - 4*a*c;
        if (D < 0) {
            // Collision impossible.
            holdFire = true;

            // Fake prediction.
            dt = gameState->timeStep;
        } else {
            holdFire = false;

            double rootD = sqrt(D);
            double dt1 = (-b + rootD) / (2 * a);
            double dt2 = (-b - rootD) / (2 * a);

            if (dt1 < 0.0 &&  dt2 < 0.0) {
                // Collision impossible.
                holdFire = true;

                // Fake prediction.
                dt = gameState->timeStep;
            } else if (dt1 >= 0.0 &&  dt2 >= 0.0){
                dt = min(dt1, dt2);
            } else {
                dt = max(dt1, dt2);
            }
        }
    }
    //Point2D nextPos = {N.x + T.x + vx * dt, N.y + T.y + vy * dt};
    Point2D nextPos = {T.x + vx * dt, T.y + vy * dt};
    //gameState->Log("nx: " + to_string(nextPos.x) +
    //               " ny: " + to_string(nextPos.y));

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

        // Do not consider my own lasers.
        Laser *obj = itr->second;
        if (obj->owner != myShip->uid) {
            // Squared speed.
            double laserSpeed2 = pow(obj->velx, 2) + pow(obj->vely, 2);

            // Fast laser beams and close ones need atention.
            if (laserSpeed2 > pow(2 * LASER_BASE_SPEED, 2) ||
                isInsideBox(obj, upperLeft, bottomRight)) {

                nearThreats.push_back(obj);
            }
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
    double Fr = 0.0;
    /*
    gameState->Log("x: " + to_string(myShip->posx) +
                   " y: " + to_string(myShip->posy));
    gameState->Log("X: " + to_string(destiny.x) +
                   " Y: " + to_string(destiny.y) +
                   " dist: " + to_string(distToDestiny));
    */

    double sinA = sin(facingAngle);
    double cosA = cos(facingAngle);

    double M = MAIN_THRUST_POWER;
    double S = SIDE_THRUST_POWER;
    double T = SIDE_THRUST_TORQUE;

    double m; // Main thrust.
    double f; // Front side thrust.
    double b; // Back side thrust.

    if (!evading) {
        Fx = -SHIP_MASS * myShip->velx * 10;
        Fy = -SHIP_MASS * myShip->vely * 10;
        Fr = T * rotationPID.compute(0.0, -angle);
    } else if (fabs(distToDestiny) > ZERO) {
        double pidDist = movementPID.compute(0.0, -distToDestiny);
        Fx = pidDist * M * (deltaX / distToDestiny);
        Fy = pidDist * M * (deltaY / distToDestiny);
        Fr = 0.0;
    }

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

/*
    if (!evading) {
        double rotationBoost = rotationPID.compute(0.0, angle);
        f += rotationBoost;
        b -= rotationBoost;
    }
*/

    double ajust = max(max(fabs(m), fabs(f)), fabs(b));
    if (ajust > 1.0) {
        m /= ajust;
        f /= ajust;
        b /= ajust;
    }
    /*
    gameState->Log("Fx: " + to_string(Fx) + " Fy: " + to_string(Fy) +
                   " a: " + to_string(ajust));
    gameState->Log("m: " + to_string(m) + " f: " + to_string(f) +
                   " b: " + to_string(b));
    */

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
        if (fabs(dist) <= ZERO) {
            Point2D v = {
                objNextPos.y - myNextPos.y,
                objNextPos.x - myNextPos.x
            };
            v.x += myNextPos.x;
            v.y += myNextPos.y;
            threatsDirections.push_back(v);
        } else if (dist <= 1.3 * (myShip->radius + obj->radius)) {
            double u = collisionDistance(myNextPos, objPos, objNextPos);
            Point2D collisionPoint = {
                objPos.x + (u + 0.02) * (objNextPos.x - objPos.x),
                objPos.y + (u + 0.02) * (objNextPos.y - objPos.y)
            };
            threatsDirections.push_back(collisionPoint);
        }
    }

    if (threatsDirections.size() == 0) {
        if (evading) {
            destiny.x = myShip->posx;
            destiny.x = myShip->posx;
        }
        evading = false;
        //gameState->Log("NOT evading");
    } else {
        evading = true;
        //gameState->Log("DO evading");
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
