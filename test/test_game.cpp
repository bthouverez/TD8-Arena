#include "common.hpp"
#include "window.hpp"
#include "shader.hpp"
#include "screenquad.hpp"
#include "renderable_asteroid.hpp"
#include "asteroid.hpp"
#include "renderable_laser.hpp"
#include "laser.hpp"
#include "tools.hpp"

#include <mat.h>
#include <color.h>

#include <string>
#include <cmath>
#include <list>



int main(void)
{
  Window * win = Window::init_fullscreen();
  int winwidth, winheight;
  win->getSize(winwidth, winheight);

  glEnable(GL_DEPTH_TEST);
  glClearDepth(1.0f);
  glClearColor(0, 0, 0, 1);

  Shader laserprogram("data/shader/laser.vertex.glsl", "data/shader/laser.fragment.glsl");
  if (!laserprogram.init())
    return -1;
  GLuint laserprog = laserprogram.getProgramID();
  
  Shader program("data/shader/default_mesh.vertex.glsl", "data/shader/default_mesh.fragment.glsl");
  if (!program.init())
    return -1;
  GLuint prog = program.getProgramID();
  
  // Transf16rms:
  Point campos(20.0,0,0);
  Transform view = Lookat(campos, Point(0,0,0), Vector(0,0,1));
  Transform projection = Perspective(50.0f, winwidth/(float)winheight, 0.01f, 100.0f);
  Transform PV = projection * view;
  Transform model;

  // Light:
  //Point lightpos(-0.2, -0.5, -1.0);
  Point lightpos(-0.2, -0.5, 15.0);
  Color lightcol(1.0, 1.0, 1.0);

  // Asteroides:
  #define NB_ASTEROIDS  16
  #define NB_ASTEROID_MODELS  7
  std::vector<RenderableAsteroid *> renderable_asteroids;
  for (int i=0; i < NB_ASTEROID_MODELS; ++i)
  {
    RenderableAsteroid * a = new RenderableAsteroid();
    a->init();
    renderable_asteroids.push_back(a);
  }

  std::list<Asteroid*> asteroids;
  for (int i=0; i < NB_ASTEROIDS; ++i)
  {
    Asteroid * a = new Asteroid;
    a->init();

    const RenderableAsteroid * ra = renderable_asteroids[rand()%renderable_asteroids.size()];    
    Point pos(getRandomFloat(-10.0f, 0.0f), getRandomFloat(-16.0f, 16.0f), getRandomFloat(-3.0f, 3.0f));

    a->setRenderableEntityID(ra->getID());        
    a->setPosition(pos);    

    asteroids.push_back(a);
  }

  // Lasers:
  std::vector<RenderableLaser*> renderable_lasers;
  RenderableLaser * laser_green = new RenderableLaser;
  laser_green->init(Color(0,1,0));
  renderable_lasers.push_back(laser_green);
  RenderableLaser * laser_red = new RenderableLaser;
  laser_red->init(Color(1,0,0));
  renderable_lasers.push_back(laser_red);
  
  std::list<Laser*> lasers;
  for (int i=0; i < 8; ++i)
  {
    Laser * laser = new Laser;
    laser->init();
    laser->setRenderableEntityID(renderable_lasers[rand() %2]->getID());
    laser->setLength(4.0f);
    laser->setSpeed(0.05f);
    laser->setPosition(Point(getRandomFloat(-10.0f, 0.0f), getRandomFloat(-16.0f, 16.0f), getRandomFloat(-3.0f, 3.0f)));
    laser->setMovingDirection(Vector(getRandomFloat(-1.0f, 1.0f), getRandomFloat(-1.0f, 1.0f), getRandomFloat(-1.0f, 1.0f)));
    lasers.push_back(laser);    
  }

  // mainloop:
  float framecount = 0.0f;
  while(win->isActive())
  {
    // UPDATE GAME STATE:
    framecount += 1.0f;    

    // Explode random asteroid
    if ((int)framecount % 1024 == 128)
    {
      int i = rand() % asteroids.size();
      std::list<Asteroid*>::iterator it = asteroids.begin();
      for (int j=0; j < i; ++j)
        it++;

      Asteroid * a = *it;
      asteroids.erase(it);

      int numa = 3 + rand() % 5;
      for (int j=0; j < numa; ++j)
      {
        Asteroid * suba = new Asteroid;
        suba->init();

        const RenderableAsteroid * ra = renderable_asteroids[rand()%renderable_asteroids.size()];
        suba->setRenderableEntityID(ra->getID()); 
        
        Vector direction(getRandomFloat(-1.0f, 1.0f), getRandomFloat(-1.0f, 1.0f), getRandomFloat(-1.0f, 1.0f));        
        
        suba->setPosition(a->getPosition());
        suba->setMovingDirection(direction);
        suba->setSpeed(getRandomFloat(0.0f, 0.02f));
        suba->setScale(a->getScale() / getRandomFloat(2.0f, 4.0f));

        asteroids.push_back(suba);
      }

      delete a;
    }

    for (auto a : asteroids)
    {      
      a->update(100.0f*1.0f/60.0f);
    }
    for (auto l : lasers)
    {      
      l->update(100.0f*1.0f/60.0f);
    }


    // REN2ER GAME STATE:
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
    glUseProgram(prog);
    glUniformMatrix4fv(glGetUniformLocation(prog, "PV"), 1, GL_TRUE, PV.buffer());
    glUniform3f(glGetUniformLocation(prog, "cameraPosition"), campos.x, campos.y, campos.z);
    glUniform3f(glGetUniformLocation(prog, "lightPosition"), lightpos.x, lightpos.y, lightpos.z);
    glUniform3f(glGetUniformLocation(prog, "lightColor"), lightcol.r, lightcol.g, lightcol.b);

    for (auto a: asteroids)
    {      
      glUniformMatrix4fv(glGetUniformLocation(prog, "Model"), 1, GL_TRUE, a->getModelMatrix().buffer());
      uint64 renderable_id = a->getRenderableEntityID();      
      for (int j=0; j < (int)renderable_asteroids.size();++j)
        if (renderable_asteroids[j]->getID() == renderable_id)
        {
          renderable_asteroids[j]->draw();          
          break;
        }
    }

    // render lasers:
    glUseProgram(laserprog);
    glUniformMatrix4fv(glGetUniformLocation(laserprog, "PV"), 1, GL_TRUE, PV.buffer());
    for (auto laser: lasers)
    {
      Point lasercenter = laser->getPosition();
      Vector laserdir = laser->getMovingDirection();
      float laserlength = laser->getLength();
      glUniform3f(glGetUniformLocation(laserprog, "laserCenterPosition"), lasercenter.x, lasercenter.y, lasercenter.z);
      glUniform3f(glGetUniformLocation(laserprog, "laserDirection"), laserdir.x, laserdir.y, laserdir.z);
      glUniform1f(glGetUniformLocation(laserprog, "laserLength"), laserlength);

      uint64 renderable_id = laser->getRenderableEntityID();      
      for (int j=0; j < (int)renderable_lasers.size();++j)
        if (renderable_lasers[j]->getID() == renderable_id)
        {          
          renderable_lasers[j]->draw();          
          break;
        } 
    }

    // swap graphics buffers:
    win->refresh();        
  }



  //// CLEANUP
  // -------------
  program.destroy();
  for (int i=0; i < (int)renderable_asteroids.size(); ++i){
    renderable_asteroids[i]->release();
    delete renderable_asteroids[i];
  }
  for (auto a: asteroids){
    a->release();
    delete a;
  }

  for (auto rl: renderable_lasers)
  {
    rl->release();
    delete rl;
  }
  for (auto l: lasers){
    l->release();
    delete l;
  }

  return 0;
}
