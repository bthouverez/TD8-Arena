#ifndef SHIP_HEADER
#define SHIP_HEADER

#include "game_entity.hpp"


class Ship: public GameEntity
{
public:
  Asteroid() : GameEntity() {}
  ~Asteroid() {}

  virtual void init() {}
  virtual void update(float dt){}
  virtual void release() {}

protected:
  float time_since_fired; // temps écoulé depuis le dernier tir (pour gérer le re-chargement de l'arme, non instantané)
};
#endif
