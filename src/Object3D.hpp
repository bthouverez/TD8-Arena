#ifndef OBJECT_3D_HPP
#define OBJECT_3D_HPP

#include "gkit/vec.h"

typedef unsigned long long uint64;



class Object3D
{
public:
  inline Object3D() {}
  ~Object3D();
    
  virtual void draw() = 0;
  virtual void update(float dt) = 0;

protected:
  Point position;  
  Vector orientation;
  float speed, acceleration;

  GLuint vao, vbo, texture;

  uint64 id = 0;
};

#endif
