#ifndef VKDrone_H
#define VKDrone_H

#define PI (3.14159265f)

#include <map>

#include "BotBase.h"
#include "bot_interface.h"

class VKDrone : public BotBase {
  public:
    VKDrone();
    virtual ~VKDrone();
    virtual void Process();

  protected:
    // Return uid of closest enemy ship or NULL when no enemy was found.
    Ship* closestShip(std::map<int, Ship*> &ships);

    // Rotates the ship to face a object or a specific coordinate.
    void aimAt(const GameObject *obj);
    void aimAt(float x, float y);
};

#endif
