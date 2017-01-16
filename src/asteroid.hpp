#ifndef ASTEROID_HEADER
#define ASTEROID_HEADER

#include "game_entity.hpp"
#include "tools.hpp"

#include <vec.h>
#include <mat.h>


class Asteroid: public GameEntity
{
public:
  Asteroid() : GameEntity() {}
  ~Asteroid() {}

  virtual void init() 
  {
    m_rotation_axis = normalize(Vector(getRandomFloat(-1,1), getRandomFloat(-1,1), getRandomFloat(-1,1)));
    m_rotation_speed = getRandomFloat(0, 3);
    m_radius = getRandomFloat(0.5, 3.0);
  }

  virtual void update(float dt)
  {
    m_rotation_axis = normalize( m_rotation_axis + 0.5f * Vector(getRandomFloat(-1,1), getRandomFloat(-1,1), getRandomFloat(-1,1)) );

    //todo: 
    //m_l2w = Translation(m_speed * orientation) * m_l2w * Rotation(axis, speed);
  }

  virtual void release() {}


protected:
  Vector m_rotation_axis;
  float m_rotation_speed;
  float m_radius;
};
#endif
