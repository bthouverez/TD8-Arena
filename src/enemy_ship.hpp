#ifndef ENNEMY_SHIP_HEADER
#define ENNEMY_SHIP_HEADER

#include "ship.hpp"


class EnnemyShip: public Ship
{
public:
  EnnemyShip() : Ship(), m_hullpoints(64) {}
  ~EnnemyShip() {}

  virtual void init() {}
  virtual void update(float dt){}
  virtual void release() {}

protected:
  
};
#endif
