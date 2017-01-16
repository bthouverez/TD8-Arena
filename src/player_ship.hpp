#ifndef PLAYER_SHIP_HEADER
#define PLAYER_SHIP_HEADER

#include "ship.hpp"


class PlayerShip: public Ship
{
public:
  PlayerShip() : Ship() {}
  ~PlayerShip() {}

  virtual void init() {}
  virtual void update(float dt){}
  virtual void release() {}

protected:
  
};
#endif
