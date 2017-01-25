#include "common.hpp"
#include "window.hpp"
#include "shader.hpp"
#include "screenquad.hpp"
#include "leapinputreader.h"
#include "game_entity.hpp"
#include "renderable_entity.hpp"

#include <string>

int main(void)
{
  Window * win = Window::init(1024, 800);
  LeapInputReader lir;
  //CameraArena cam = CameraArena(1920, 1080, 60, 1);

  RenderableEntity renderable;
  renderable.loadOBJ("data/object/tie.obj");
  GameEntity ship;
  ship.setRenderableEntityID(renderable.getID());

  //glEnable(GL_DEPTH_TEST);
  glDisable(GL_DEPTH_TEST);
  glClearDepth(1.0f);
  glClearColor(0, 0, 0, 1);

  ScreenQuad quad;
  quad.init();
  quad.loadTexture(std::string("data/texture/test.jpg"), GL_RGB8);

  Shader program("data/shader/screenquad.vertex.glsl", "data/shader/screenquad.fragment.glsl");
  if (!program.init())
    return -1;
  GLuint prog = program.getProgramID();
  glUseProgram(prog);

  while(win->isActive())
  {
    lir.Update();
    
    float Height =  lir.GetHeight();
    float Speed =  lir.GetSpeed();
    float Direction =  lir.GetDirection();
    bool Shoot =  lir.GetShoot();

    /*std::cout << 
     "Height " << Height << std::endl <<
     "Speed " << Speed << std::endl <<
     "Direction " << Direction << std::endl <<
     "Shoot " << Shoot << std::endl << std::endl;*/

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, quad.getTexture());
    glUniform1i(glGetUniformLocation(prog, "image"), 0);


    quad.draw();
    win->refresh();
  }

  program.destroy();
  quad.release();

  return 0;
}
