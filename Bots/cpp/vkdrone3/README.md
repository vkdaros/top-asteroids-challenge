# VKDrone3

This VKDrone version tries to evade from nearby asteroids and laser beams. At
same time, it aims at the place where the closest enemy is going to be (as
VKDrone2).  
Checking if every threat in arena is in collision route with the ship is
expensive. To avoid that cost, the bot first make a list of objects inside a
'nearby' AABB (which is cheap) and only do heavy calculation with the elements
in the list.

A simplified Makefile (tested only on Linux) can be found in **src** folder. But
you can generate a better one using CMake (recommended). The commands to do that
and compile are:
```
cd build/
cmake ..
make
```
