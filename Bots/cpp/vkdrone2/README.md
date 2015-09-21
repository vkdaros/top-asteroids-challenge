# VKDrone2

This VKDrone version just tries to shoot at the place where the closest enemy is
going to be. <br />
It is based on VKDrone1, but only rotates without moving to other place.

The drone pick select next shot charge at random (1 or 2) and keeps charging
until that power level is reached. Once it is done, VKDrone2 only fires if it is
aligned with the intended direction. <br />
Eventually this waiting process may allow the charge power reach an upper level.
In that case, the faster laser beam is used.

A simplified Makefile (tested only on Linux) can be found in **src** folder. But
you can generate a better one using CMake (recommended). The commands to do that
and compile are:
```
cd build/
cmake ..
make
```
