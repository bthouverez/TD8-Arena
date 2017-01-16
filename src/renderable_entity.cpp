#include "renderable_entity.hpp"

#include <wavefront.h>
#include <mesh.h>
#include <vec.h>


bool RenderableEntity::loadOBJ(const std::string & filename)
{
  Mesh mesh = read_mesh(filename.c_str());

  if (mesh.index_count() > 0)
    m_useindex = true;
  if (mesh.texcoords().size() > 0)
    m_usetexture = true;

  m_num_vertices = mesh.vertex_count();

  // GL:
  if (m_useindex)
  {
    m_num_indices = mesh.index_buffer_size();
    glGenBuffers(1, &m_ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_num_indices * sizeof(unsigned int), mesh.index_buffer(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  }

  glGenVertexArrays(1, &m_vao)
  glBindVertexArray(m_vao);

  glGenBuffers(1, &m_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  if (m_usetexture)
       glBufferData(GL_ARRAY_BUFFER, 2*m_num_vertices * sizeof(vec3) + m_num_vertices *sizeof(vec2), 0, GL_STATIC_DRAW);
  else glBufferData(GL_ARRAY_BUFFER, 2*m_num_vertices * sizeof(vec3), 0, GL_STATIC_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER, 0, m_num_vertices * sizeof(vec3), mesh.vertex_buffer());//coords
  glBufferSubData(GL_ARRAY_BUFFER, m_num_vertices* sizeof(vec3), m_num_vertices*sizeof(vec3), mesh.normal_buffer());//normales
  if (m_usetexture)
      glBufferSubData(GL_ARRAY_BUFFER, 2*m_num_vertices *sizeof(vec3), m_num_vertices*sizeof(vec2), mesh.texcoord_buffer());//texcoords
  
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);//coords
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(m_num_vertices*sizeof(vec3)));//normales
  glEnableVertexAttribArray(1);
  if (m_usetexture)
  {
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(2*m_num_vertices*sizeof(vec3)));//texcoords
    glEnableVertexAttribArray(2);
  }
  if (m_useindex)
  {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
  }

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

