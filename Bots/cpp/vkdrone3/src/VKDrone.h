#ifndef VKDrone_H
#define VKDrone_H

#include <map>
#include <vector>

#include "BotBase.h"
#include "bot_interface.h"
#include "PID.h"
#include "Point2D.h"

#define PI (3.14159265)
#define ZERO (0.001)

#define LASER_BASE_SPEED (25.0)
#define NEAR_DIST (25)

#define MAIN_THRUST_POWER  (30) // Newtons
#define SIDE_THRUST_POWER  (15)
#define SIDE_THRUST_TORQUE (30)

// Rotation PID gain constants.
#define KP_ROT (0.4)
#define KI_ROT (0.0)
#define KD_ROT (0.25)

// Movement PID gain constants.
#define KP_MOVE (2.0)
#define KI_MOVE (0.0)
#define KD_MOVE (2.5)

using std::map;
using std::vector;

class VKDrone : public BotBase {
  public:
    VKDrone();
    virtual ~VKDrone();
    virtual void Process();


  protected:
    // Corrected angle from -PI to PI.
    // North = 0; West = PI/2; South = +/-PI; East = -PI/2.
    double facingAngle;


    // Return uid of closest object or NULL when no one was found.
    template <typename T> T* closestObject(map<int, T*> &objects);
    GameObject* closestObject(std::map<int, GameObject*> &objects);

    // Rotates the ship to face a object or a specific coordinate.
    // Returns angle [0, PI] between myShip direction and obj position.
    double angleTo(const GameObject *obj);
    double angleTo(double x, double y);
    double angleTo(Point2D point);

    // Returns expected position of the object at next time step.
    Point2D futurePosition(const GameObject *obj, double charge);


  private:
    PID rotationPID;
    PID movementPID;

    // Indicates the power of next shoot. If charge == -1, then a new power
    // value is going to be given by rand().
    int charge;

    // List of rocks ans lasers near the ship.
    vector<GameObject*> nearThreats;


    void updateFacingAngle();

    // Fill threats list with rocks and lasers inside a bound box centered in
    // the ship and with (2 * nearDist) side length.
    void updateNearThreats(double nearDist);

    void goTo(Point2D destiny, double angle);
};

// Template definition.

template <typename T> inline
T* VKDrone::closestObject(map<int, T*> &objects) {
    // Squared distance to closest threat.
    double shortestDist2 = -1;

    // ID of closest object.
    // Initial value just to check later if any other object was found.
    int uid = myShip->uid;

	for (typename map<int, T*>::iterator itr = objects.begin();
         itr != objects.end(); itr++) {

        T *obj = itr->second;

        // Ignore myShip.
        if (obj->uid == myShip->uid) {
            continue;
        }

        // Squared distance to enemy ship.
        double dist2 = pow(obj->posx - myShip->posx, 2) +
                       pow(obj->posy - myShip->posy, 2);

        // Update closest enemy information.
        if (dist2 < shortestDist2 || shortestDist2 <= 0) {
            shortestDist2 = dist2;
            uid = obj->uid;
        }
    }

    if (uid == myShip->uid) {
        // No enemy found.
        return NULL;
    }
    return objects[uid];
}


/*******************************************************************************
 * "Hidden" auxiliary functions.
 * It shuld be refactored.
 ******************************************************************************/

bool isInsideBox(GameObject *obj, Point2D upperLeft, Point2D bottomRight);

#endif