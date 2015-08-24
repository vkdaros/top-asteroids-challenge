#ifndef VKDrone_H
#define VKDrone_H

#include <map>

#include "BotBase.h"
#include "bot_interface.h"

class VKDrone : public BotBase {
  public:
    VKDrone();
    virtual ~VKDrone();
    virtual void Process();

  protected:
    // Return uid of closest enemy ship.
    int closestShip(std::map<int, Ship*> &ships);
};

#endif
