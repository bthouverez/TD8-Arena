#ifndef GAME_ENTITY_HPP
#define GAME_ENTITY_HPP

#include "common.hpp"
#include <vec.h>
#include <mat.h>


class GameEntity: public Common
{
public:

  GameEntity() : Common() {}

  virtual ~GameEntity(){ }

  void setRenderableEntityID(uint64 id) { m_renderable = id; }
  uint64 getRenderableEntityID() { return m_renderable; }

  virtual bool init() { return true; }
  virtual void update(float dt) = 0;
  virtual void release() {};  

  
  void getBoundingBox(Point & pmin, Point & pmax) const { pmin = m_pmin; pmax = m_pmax; }

  void setSpeedMax(float smax) { m_speed_max = smax; }

  void incrSpeed(float incr)
  {
    m_speed += incr;
    if (m_speed > m_speed_max) m_speed = m_speed_max;
    if (m_speed < 0.0f) m_speed = 0.0f;
  }

protected:

  Transform m_l2w; 
  Point m_position;  
  Vector m_orientation;
  Point m_pmin, m_pmax; // boundingbox;
  float m_speed, m_speed_max;  

  uint64 m_renderable = 0;
};

#endif
