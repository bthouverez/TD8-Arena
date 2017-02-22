#ifndef LASER_HEADER
#define LASER_HEADER

#include "game_entity.hpp"



class Laser: public GameEntity
{
public:
  Laser(): GameEntity() {}
  virtual ~Laser() {}

  void setLength(float l){ length = l; }
  
  float getLength() const { return length; }


private:
  float length = 1.0f;

};


#endif