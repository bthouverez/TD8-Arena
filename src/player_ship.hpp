#ifndef PLAYER_SHIP_HEADER
#define PLAYER_SHIP_HEADER

#include "ship.hpp"


class PlayerShip: public Ship
{
public:
  PlayerShip() : Ship(), m_hullpoints(256) {}
  ~PlayerShip() {}

  
protected:
  
};
#endif
