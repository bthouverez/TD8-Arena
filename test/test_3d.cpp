#include "window.hpp"
#include "common.hpp"
#include "shader.hpp"
#include "screenquad.hpp"

#include <string>

int main(void)
{
  Window::init(1366, 800);

  glEnable(GL_DEPTH_TEST);
  glClearDepth(1.0f);
  glClearColor(0, 0, 0, 1);

  ScreenQuad quad;
  quad.init();
  quad.loadTexture(std::string("data/texture/test.jpg"), GL_RGB8);

  Shader program("data/shader/screenquad.vertex.glsl", "data/shader/screenquad.fragment.glsl");
  if (!program.init())
    return -1;
  GLuint prog = program.getProgramID();

  while(Window::isActive())
  {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(prog);
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, quad.getTexture());
    glUniform1i(glGetUniformLocation(prog, "image"), 0);

    quad.draw();

    Window::refresh();
  }

  program.destroy();
  quad.release();
}