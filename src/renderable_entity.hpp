#ifndef RENDERABLE_ENTITY_HEADER
#define RENDERABLE_ENTITY_HEADER

#include "common.hpp"

#include <string>



class RenderableEntity : public Common
{
public:

  RenderableEntity() : Common() {}
  virtual ~RenderableEntity() { release(); }

  virtual bool init() { return true; }

  virtual void draw() = 0;

  virtual void release() { }

  bool loadOBJ(const std::string & filename);  


protected:
  GLuint m_vao, m_vbo, m_ibo = 0, m_texture = 0;
  int m_num_indices = -1, m_num_vertices;
  bool m_usetexture = false, m_useindex = false;
};
#endif
