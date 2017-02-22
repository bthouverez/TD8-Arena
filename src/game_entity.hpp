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
  
  uint64 getRenderableEntityID() const { return m_renderable; }

  virtual bool init()
  {
    m_position = Point(0,0,0);
    m_direction = Vector(1,0,0);
    m_orientation = Identity();
    m_speed = 0;
    m_speed_max = 10.0f;
    m_scale = 1;
    return true;
  }
  
  virtual void update(float dt) { updatePosition(dt); }
  
  virtual void release() {}


  void setPosition(const Point & p) { m_position = p; }

  const Point & getPosition() const { return m_position; }
  
  void setMovingDirection(const Vector & v) { m_direction = normalize(v); }

  const Vector & getMovingDirection() const { return m_direction; }

  const Transform & getOrientation() const { return m_orientation; }

  void setOrientation(const Transform & o){ m_orientation = o; }

  void setSpeed(float s) { m_speed = s; }

  void setMaxSpeed(float smax) { m_speed_max = smax; }

  void accelerate(float speed_increment)
  {
    m_speed += speed_increment;
    if (m_speed > m_speed_max) m_speed = m_speed_max;
    if (m_speed < 0.0f) m_speed = 0.0f;
  }

  //! @param changeDirection Si vrai la rotation s'applique aussi à la direction de mouvement courante de l'entité
  void rotate(const Vector & axis, float angle_degree, bool changeDirection = true)
  {
    Transform rot = Rotation(axis, angle_degree);
    m_orientation = rot * m_orientation;
    if (changeDirection)
      m_direction = rot(m_direction);
  }
  void rotateX(float angle_degree, bool changeDirection = true)
  {
    Transform rot = RotationX(angle_degree);
    m_orientation = rot * m_orientation;
    if (changeDirection)
      m_direction = rot(m_direction);
  }
  void rotateY(float angle_degree, bool changeDirection = true)
  {
    Transform rot = RotationY(angle_degree);
    m_orientation = rot * m_orientation;
    if (changeDirection)
      m_direction = rot(m_direction);
  }
  void rotateZ(float angle_degree, bool changeDirection = true)
  {
    Transform rot = RotationZ(angle_degree);
    m_orientation = rot * m_orientation;
    if (changeDirection)
      m_direction = rot(m_direction);
  }

  Transform getModelMatrix() const { return Translation(Vector(m_position)) * m_orientation * Scale(m_scale, m_scale, m_scale); }

  void setScale(float s) { m_scale = s; }

  float getScale() const { return m_scale; }
  
  void setBoundingRadius(float radius){ m_boundingRadius = radius; }

  float getBoundingRadius() const { return m_boundingRadius; }


  /** Test if a point collides with bounding radius : point is given in world coordinates */
  bool collidePoint(const Point &p) const
  {    
    float d = distance(m_position, p);
    return d < m_boundingRadius;
  }


protected:

  virtual void updatePosition(float dt)
  {    
    Transform t = Translation(dt*m_speed*(m_direction));
    m_position = t(m_position);
  }

  Transform m_orientation;// rotation matrix
  Vector m_direction; // current moving direction
  Point m_position;// 3D position  
  float m_boundingRadius = 0.0f; // bounding sphere
  float m_scale;// 3D scale
  float m_speed, m_speed_max;  

  uint64 m_renderable = 0;
};

#endif
