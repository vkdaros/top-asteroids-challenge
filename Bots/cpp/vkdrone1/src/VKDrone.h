#ifndef VKDrone_H
#define VKDrone_H

#include <map>

#include "BotBase.h"
#include "bot_interface.h"
#include "PID.h"

#define PI (3.14159265)

// Rotation PID gain constants.
#define KP_ROT (0.45)
#define KI_ROT (0.0)
#define KD_ROT (0.25)

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
    void aimAt(double x, double y);

  private:
    PID rotationPID;
};

#endif
