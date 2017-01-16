#ifndef OBJECT_3D_HPP
#define OBJECT_3D_HPP

#include "common.hpp"
#include <vec.h>
#include <mat.h>


class Object3D: public Common
{
public:

  Object3D() : Common() {}

  virtual ~Object3D(){ release(); }


  virtual bool init() = 0;
  virtual void draw() = 0;
  virtual void update(float dt) = 0;
  virtual void release() = 0;  


  bool loadOBJ(const std::string & filename);
  
  void getBoundingBox(Point & pmin, Point & pmax) { pmin = this->pmin; pmax = this->pmax; }

  void setSpeedMax(float sm) { speedMax = sm; }

  void incrSpeed(float incr)
  {
    speed += incr;
    if (speed > speedMax) speed = speedMax;
    if (speed < 0.0f) speed = 0.0f;
  }

protected:

  Transform l2w;
  Point position;  
  Vector orientation;

  Point pmin, pmax; // boundingbox;
  
  float speed, speedMax;

  GLuint vao = 0, vbo = 0, texture = 0;
};

#endif
