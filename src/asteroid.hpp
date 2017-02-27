#ifndef ASTEROID_HEADER
#define ASTEROID_HEADER

#include "game_entity.hpp"
#include "tools.hpp"

#include <vec.h>
#include <mat.h>


class Asteroid: public GameEntity
{
public:
  Asteroid() : GameEntity() {}
  ~Asteroid() {}

  virtual bool init()
  {
    m_rotation_axis = normalize(Vector(getRandomFloat(-1,1), getRandomFloat(-1,1), getRandomFloat(-1,1)));
    m_rotation_speed = getRandomFloat(0, 0.2);
    m_scale = getRandomFloat(0.68, 2.7); 
    m_boundingRadius = 1.0f;    

    return true;
  }

  virtual void update(float dt)
  {    
    //m_rotation_axis = normalize( m_rotation_axis + 0.01f * Vector(getRandomFloat(-1,1), getRandomFloat(-1,1), getRandomFloat(-1,1)) );        
    rotate(m_rotation_axis, dt * m_rotation_speed, false);

    //GameEntity::update(dt);
    updatePosition(dt);    
  }

  virtual void release() {}

  float getBoundingRadius() const {
    return m_scale;
  }

protected:
  Vector m_rotation_axis;
  float m_rotation_speed;    
};
#endif
