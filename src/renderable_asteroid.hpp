#ifndef RENDERABLE_ASTEROID_HEADER
#define RENDERABLE_ASTEROID_HEADER

#include "renderable_entity.hpp"
#include <vec.h>


class RenderableAsteroid: public RenderableEntity
{
public:

  #define CUBOID_SUBDIV     32
  RenderableAsteroid() : RenderableEntity(){}
  virtual ~RenderableAsteroid() {}

  virtual bool init();

protected:
  Vector m_fbmSeed;
};

#endif
