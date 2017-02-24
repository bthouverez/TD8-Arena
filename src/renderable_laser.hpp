#ifndef RENDERABLE_LASER_HPP
#define RENDERABLE_LASER_HPP

#include "common.hpp"

#include <color.h>



class RenderableLaser: public Common
{
public:

  RenderableLaser() {}
  virtual ~RenderableLaser() {}


  bool init(const Color & color)
  {
    this->color = color;
    
    const float vertices[6] = { -1.0f, 0.0f, 0.0f, 
                                1.0f, 0.0f, 0.0f }; // 2 points (laser is x-axis aligned)
    float colors[6] = { color.r, color.g, color.b, 0.5f*(color.r + 1.0f), 0.5f*(color.g + 1.0f), 0.5f*(color.b + 1.0f)};

    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);
    
    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, (6 + 6) * sizeof(float), 0, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, 6 * sizeof(float), vertices);
    glBufferSubData(GL_ARRAY_BUFFER, 6 * sizeof(float), 6* sizeof(float), colors);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); //coords
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(6*sizeof(float)));//couleurs
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return true;
  }

  void draw() 
  {
    glEnable(GL_LINE_SMOOTH);
    glLineWidth(4.0f);    // ne semble pas fonctionner
    glBindVertexArray(m_vao);    
    glDrawArrays(GL_LINES, 0, 2);
    glBindVertexArray(0);
  }

  void release()
  {
    glDeleteVertexArrays(1, &m_vao);
    glDeleteBuffers(1, &m_vbo);
  }


private:

  Color color;
  GLuint m_vao, m_vbo;
};
#endif
