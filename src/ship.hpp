#ifndef SHIP_HEADER
#define SHIP_HEADER

#include "game_entity.hpp"


class Ship: public GameEntity
{
public:
  Ship() : GameEntity() {}
  ~Ship() {}

  void setReloadTime(float time) { m_reload_time = time; }
  bool isReloading() const { return m_time_since_fired < m_reload_time; }

  
protected:
  float m_time_since_fired; // temps écoulé depuis le dernier tir (pour gérer le re-chargement de l'arme, non instantané)
  float m_reload_time; // délai entre deux tirs
};
#endif
