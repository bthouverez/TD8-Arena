#include "renderable_entity.hpp"

#include <image.h>
#include <image_io.h>
#include <wavefront.h>
#include <mesh.h>
#include <vec.h>


void RenderableEntity::draw()
{
  glBindVertexArray(m_vao);
  if (m_useindex)
    glDrawElements(GL_TRIANGLES, m_num_indices, GL_UNSIGNED_INT, 0/* element array buffer offset*/);
  else 
    glDrawArrays(GL_TRIANGLES, 0, m_num_vertices);
  glBindVertexArray(0);
}

void RenderableEntity::release()
{
  glDeleteVertexArrays(1, &m_vao);
  glDeleteBuffers(1, &m_vbo);
  if (m_useindex)
    glDeleteBuffers(1, &m_ibo);
  if (m_usetexture)
    glDeleteTextures(1, &m_texture);
}

bool RenderableEntity::loadTexture(const std::string & filename, GLuint texel_type)
{
  ImageData image = read_image_data(filename.c_str());
  if(image.data.empty())
    return false;
    
  // cree la texture openGL
  glGenTextures(1, &m_texture);  
  glBindTexture(GL_TEXTURE_2D, m_texture);    
  // fixe les parametres de filtrage par defaut
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

  GLenum format;
  switch(image.channels)
  {
    case 1: format= GL_RED; break;
    case 2: format= GL_RG; break;
    case 3: format= GL_RGB; break;
    case 4: format= GL_RGBA; break;
    default: format= GL_RGBA; 
  }
    
  GLenum type;
  switch(image.size)
  {
    case 1: type= GL_UNSIGNED_BYTE; break;
    case 4: type= GL_FLOAT; break;
    default: type= GL_UNSIGNED_BYTE;
  }
    
  // transfere les donnees dans la texture
  glTexImage2D(GL_TEXTURE_2D, 0, texel_type, image.width, image.height, 0, format, type, image.buffer());
    
  // prefiltre la texture
  glGenerateMipmap(GL_TEXTURE_2D);

  return true;
}

bool RenderableEntity::loadOBJ(const std::string & filename)
{
  Mesh mesh = read_mesh(filename.c_str());

  if (mesh.index_count() > 0)
    m_useindex = true;
  if (mesh.texcoords().size() > 0)
    m_usetexture = true;

  m_num_vertices = mesh.vertex_count();

  // Colors (vec3 instead of vec4)
  std::vector<vec3> colors;
  for(auto color : mesh.colors())
  {
    vec3 c = vec3(color.x,color.y,color.z);
    colors.push_back(c);
  }

  // GL:
  if (m_useindex)
  {
    m_num_indices = mesh.index_buffer_size();
    glGenBuffers(1, &m_ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_num_indices * sizeof(unsigned int), mesh.index_buffer(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  }
  
  glGenVertexArrays(1, &m_vao);
  glBindVertexArray(m_vao);

  glGenBuffers(1, &m_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

  if (m_usetexture)
    glBufferData(   GL_ARRAY_BUFFER, 
                    3 * m_num_vertices * sizeof(vec3) +     // Position + Normal + Color
                    m_num_vertices * sizeof(vec2),          // Texcoord
                    0, GL_STATIC_DRAW);
  else 
    glBufferData(   GL_ARRAY_BUFFER, 
                    3 * m_num_vertices * sizeof(vec3),       // Position + Normal + Color
                    0, GL_STATIC_DRAW);

  std::cout << "vertex : " <<  mesh.vertex_count() << std::endl;
  std::cout << "normals : " <<  mesh.normals().size() << std::endl;
  std::cout << "colors : " <<  colors.size() << std::endl;
  std::cout << "faces : " <<  mesh.triangle_count() << std::endl;

  glBufferSubData(GL_ARRAY_BUFFER, 0, m_num_vertices * sizeof(vec3), mesh.vertex_buffer());//coords

  if(mesh.color_buffer_size() > 0)
      glBufferSubData(GL_ARRAY_BUFFER, m_num_vertices * sizeof(vec3), m_num_vertices * sizeof(vec3), &colors.front().x);//couleurs

  glBufferSubData(GL_ARRAY_BUFFER, 2 * m_num_vertices * sizeof(vec3), m_num_vertices * sizeof(vec3), mesh.normal_buffer());//normales

  if (m_usetexture)
      glBufferSubData(GL_ARRAY_BUFFER, 3*m_num_vertices *sizeof(vec3), m_num_vertices*sizeof(vec2), mesh.texcoord_buffer());//texcoords
  
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); //coords
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(m_num_vertices*sizeof(vec3)));//couleurs
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(2*m_num_vertices*sizeof(vec3)));//normales
  glEnableVertexAttribArray(2);
  if (m_usetexture)
  {
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(3*m_num_vertices*sizeof(vec3)));//texcoords
    glEnableVertexAttribArray(3);
  }
  if (m_useindex)
  {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
  }

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  return true;
}

