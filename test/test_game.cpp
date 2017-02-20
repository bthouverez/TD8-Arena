#include "common.hpp"
#include "window.hpp"
#include "shader.hpp"
#include "screenquad.hpp"
#include "renderable_asteroid.hpp"
#include <mat.h>
#include <color.h>

#include <string>
#include <cmath>



int main(void)
{
  Window * win = Window::init_fullscreen();
  int winwidth, winheight;
  win->getSize(winwidth, winheight);

  glEnable(GL_DEPTH_TEST);
  glClearDepth(1.0f);
  glClearColor(0, 0, 0, 1);

  Shader program("data/shader/default_mesh.vertex.glsl", "data/shader/default_mesh.fragment.glsl");
  if (!program.init())
    return -1;
  GLuint prog = program.getProgramID();
  glUseProgram(prog);

  // Transforms:
  Point campos(4.0,0,0);
  Transform view = Lookat(campos, Point(0,0,0), Vector(0,0,1));
  Transform projection = Perspective(50.0f, winwidth/(float)winheight, 0.01f, 100.0f);
  Transform PV = projection * view;
  Transform model;

  // Light:
  Point lightpos(-0.2, -0.5, -1.0);
  Color lightcol(1.0, 1.0, 1.0);

  // Asteroide:
  RenderableAsteroid ast;
  ast.init();

  // mainloop:
  float framecount = 0.0f;
  while(win->isActive())
  {
    framecount += 1.0f;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float phi = 180.0f * std::sin(0.2f*framecount/60.0f);
    float theta = 180.0f * std::sin(framecount/60.0f*0.16f);
    model = RotationZ(phi);
    model = RotationX(theta) * model; 
    glUniformMatrix4fv(glGetUniformLocation(prog, "PV"), 1, GL_TRUE, PV.buffer());
    glUniformMatrix4fv(glGetUniformLocation(prog, "Model"), 1, GL_TRUE, model.buffer());
    glUniform3f(glGetUniformLocation(prog, "cameraPosition"), campos.x, campos.y, campos.z);
    glUniform3f(glGetUniformLocation(prog, "lightPosition"), lightpos.x, lightpos.y, lightpos.z);
    glUniform3f(glGetUniformLocation(prog, "lightColor"), lightcol.r, lightcol.g, lightcol.b);

    ast.draw();
    win->refresh();
  }

  program.destroy();
  ast.release();
  //quad.release();

  return 0;
}
