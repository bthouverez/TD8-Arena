#include "common.hpp"
#include "window.hpp"
#include "shader.hpp"
#include "screenquad.hpp"
#include "renderable_asteroid.hpp"
#include "asteroid.hpp"
#include "tools.hpp"

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
  Point campos(20.0,0,0);
  Transform view = Lookat(campos, Point(0,0,0), Vector(0,0,1));
  Transform projection = Perspective(50.0f, winwidth/(float)winheight, 0.01f, 100.0f);
  Transform PV = projection * view;
  Transform model;

  // Light:
  Point lightpos(-0.2, -0.5, -1.0);
  Color lightcol(1.0, 1.0, 1.0);

  // Asteroides:
  #define NB_ASTEROIDS  24
  #define NB_ASTEROID_MODELS  7
  std::vector<RenderableAsteroid *> renderable_asteroids;
  for (int i=0; i < NB_ASTEROID_MODELS; ++i)
  {
    RenderableAsteroid * a = new RenderableAsteroid();
    a->init();
    renderable_asteroids.push_back(a);
  }

  std::vector<Asteroid*> asteroids;
  for (int i=0; i < NB_ASTEROIDS; ++i)
  {
    Asteroid * a = new Asteroid;
    const RenderableAsteroid * ra = renderable_asteroids[rand()%renderable_asteroids.size()];
    a->setRenderableEntityID(ra->getID());    
    a->init();
    Point pos(getRandomFloat(-10.0f, 0.0f), getRandomFloat(-16.0f, 16.0f), getRandomFloat(-3.0f, 3.0f));
    a->setPosition(pos);

    asteroids.push_back(a);
  }


  // mainloop:
  float framecount = 0.0f;
  while(win->isActive())
  {
    // UPDATE GAME STATE:
    framecount += 1.0f;
    for (int i=0; i < (int)asteroids.size(); ++i)
    {      
      asteroids[i]->update(100.0f*1.0f/60.0f);
    }

    // RENDER GAME STATE:
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
    glUniformMatrix4fv(glGetUniformLocation(prog, "PV"), 1, GL_TRUE, PV.buffer());
    glUniform3f(glGetUniformLocation(prog, "cameraPosition"), campos.x, campos.y, campos.z);
    glUniform3f(glGetUniformLocation(prog, "lightPosition"), lightpos.x, lightpos.y, lightpos.z);
    glUniform3f(glGetUniformLocation(prog, "lightColor"), lightcol.r, lightcol.g, lightcol.b);

    for (int i=0; i < (int)asteroids.size(); ++i)
    {      
      glUniformMatrix4fv(glGetUniformLocation(prog, "Model"), 1, GL_TRUE, asteroids[i]->getModelMatrix().buffer());
      uint64 renderable_id = asteroids[i]->getRenderableEntityID();      
      for (int j=0; j < (int)renderable_asteroids.size();++j)
        if (renderable_asteroids[j]->getID() == renderable_id)
        {
          renderable_asteroids[j]->draw();          
          break;
        }
    }

    // swap graphics buffers:
    win->refresh();    
  }

  program.destroy();
  for (int i=0; i < (int)renderable_asteroids.size(); ++i){
    renderable_asteroids[i]->release();
    delete renderable_asteroids[i];
  }
  for (int i=0; i < (int)asteroids.size(); ++i){
    asteroids[i]->release();
    delete asteroids[i];
  }

  return 0;
}
