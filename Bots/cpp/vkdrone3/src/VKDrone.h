#ifndef VKDrone_H
#define VKDrone_H

#include <map>

#include "BotBase.h"
#include "bot_interface.h"
#include "PID.h"
#include "Point2D.h"

#define PI (3.14159265)

#define LASER_BASE_SPEED (25.0)

// Rotation PID gain constants.
#define KP_ROT (0.4)
#define KI_ROT (0.0)
#define KD_ROT (0.25)

using std::map;

class VKDrone : public BotBase {
  public:
    VKDrone();
    virtual ~VKDrone();
    virtual void Process();

  protected:
    // Return uid of closest object or NULL when no one was found.
    template <typename T> T* closestObject(map<int, T*> &objects);
    GameObject* closestObject(std::map<int, GameObject*> &objects);

    // Rotates the ship to face a object or a specific coordinate.
    // Returns angle [0, PI] between myShip direction and obj position.
    double aimAt(const GameObject *obj);
    double aimAt(double x, double y);
    double aimAt(Point2D point);

    // Returns expected position of the object at next time step.
    Point2D futurePosition(const GameObject *obj, double charge);

  private:
    PID rotationPID;

    // Indicates the power of next shoot. If charge == -1, then a new power
    // value is going to be given by rand().
    int charge;
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

#endif
