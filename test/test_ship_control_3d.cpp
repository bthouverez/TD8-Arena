#include "common.hpp"
#include "window.hpp"
#include "shader.hpp"
#include "cameraarena.hpp"
#include "screenquad.hpp"
#include "leapinputreader.h"
#include "game_entity.hpp"
#include "renderable_entity.hpp"
#include "chessboard.hpp"
#include "renderable_asteroid.hpp"
#include "asteroid.hpp"
#include "renderable_laser.hpp"
#include "laser.hpp"

#include <string>
#include <vector>
#include <list>

//////////// GLOBALS ////////////

float GAME_SCALE;

/////////// Fwdcl //////////////

void explodeAsteroid(std::list<Asteroid*>::iterator & it, std::list<Asteroid*> & asteroids, const std::vector<RenderableAsteroid*> & renderable_asteroids);

void updateLasers(std::list<Laser*> & lasers, const Transform & PV);

void shipCollide(GameEntity ** ship, uint64 id);


/////////// MAIN ///////////////

int main(int argc, char** argv)
{
  ////////// Parameters //////////

  if(argc < 8 || argc > 9)
  {
    std::cerr << "Usage : bin/calib <width> <height> <rate>";
    std::cerr << " <chessboard_width> <chessboard_height> <square_size> <camera_parameters_file> [<device>]" << std::endl;
    exit(1);
  }

  int width = atoi(argv[1]);
  int height = atoi(argv[2]);
  float rate = atof(argv[3]);
  int chess_width = atoi(argv[4]);
  int chess_height = atoi(argv[5]);
  int chess_size = atoi(argv[6]);
  int device = argc == 9 ? atoi(argv[8]) : 0;

  // Echelle du jeu en fonction de la taille de la mire:
  GAME_SCALE = chess_size * 0.5f;


  ////////// GL Window //////////

  Window * win = Window::init(width, height);

  ////////// Leap Joystick //////////

  LeapInputReader lir;
  
  ////////// Camera //////////

  CameraArena cam = CameraArena(width, height, rate, device);
  // Load intrinsics parameters
  cam.read(argv[7]);
  // Calc extrinsics parameters
  int cmp = 0;
  do
  {
    std::cout << "Camera calibration ..." << std::endl;
    cam.get();
    cmp++;
  }
  while(cmp < 16 or !cam.extrinsics(chess_width, chess_height, chess_size));
  cam.frustum( width, height );

  
  glm::mat4 PV = cam.projection() * cam.view() * cam.gtoc();
  Transform transform_PV(
    PV[0][0], PV[1][0], PV[2][0], PV[3][0],
    PV[0][1], PV[1][1], PV[2][1], PV[3][1],
    PV[0][2], PV[1][2], PV[2][2], PV[3][2],
    PV[0][3], PV[1][3], PV[2][3], PV[3][3]
  );

  ////////// Ship //////////

  RenderableEntity renderable_ship;
  renderable_ship.loadOBJ("data/object/tie.obj");
  GameEntity * ship = new GameEntity();
  ship->init();
  ship->setRenderableEntityID(renderable_ship.getID());
  ship->setPosition(Point(6.0 * GAME_SCALE,4.0 * GAME_SCALE,-2*GAME_SCALE));// position de départ du vaisseau
  ship->rotateX(90.0f, false);// orientation de départ
  ship->setMovingDirection(Vector(0.0f, -1.0f, 0.0f));
  ship->setScale(GAME_SCALE);

  ////////// Lasers //////////
  std::vector<RenderableLaser*> renderable_lasers;
  RenderableLaser * laser_green = new RenderableLaser;
  laser_green->init(Color(0,1,0));
  renderable_lasers.push_back(laser_green);
  /*RenderableLaser * laser_red = new RenderableLaser;
  laser_red->init(Color(1,0,0));
  renderable_lasers.push_back(laser_red);// unused! */
  
  std::list<Laser*> lasers;

  ////////// ASTEROID //////////

  #define NB_ASTEROIDS  32
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
    Point pos(GAME_SCALE * getRandomFloat(-30.0f, 30.0f), GAME_SCALE * getRandomFloat(-120.0f, 0.0f), -GAME_SCALE * getRandomFloat(0.0f, 6.0f));

    a->setRenderableEntityID(ra->getID());        
    a->setPosition(pos);  
    a->setScale(3.0f*GAME_SCALE);  

    asteroids.push_back(a);
  }

  ////////// GL Parameters //////////

  glEnable(GL_DEPTH_TEST);
  glClearColor(0, 0, 0, 1);
  glClearDepth(1.0f);

  ////////// Background init //////////

  ScreenQuad quad;
  quad.init();

  Chessboard chess;
  chess.init(chess_width, chess_height, chess_size);
  chess.loadTexture("data/texture/floor.jpg", GL_RGB8);

  ////////// Shaders init //////////

  Shader laserprogram("data/shader/laser.vertex.glsl", "data/shader/laser.fragment.glsl");
  if (!laserprogram.init())
    return -1;
  GLuint laserprog = laserprogram.getProgramID();
  Shader background_program("data/shader/screenquad.vertex.glsl", "data/shader/screenquad.fragment.glsl");
  Shader renderable_program("data/shader/default_mesh.vertex.glsl", "data/shader/default_mesh.fragment.glsl");
  if (!background_program.init() || !renderable_program.init())
  {
    std::cout << "Can not load shader programs ..." << std::endl;
    exit(1);
  }

  int cpt_laser = 50;

  while(win->isActive())
  {
  ////////// Joystick values //////////


    lir.Update();   

    if(lir.GetHand()) {
       float Height =  lir.GetHeight();
      float Speed =  lir.GetSpeed();
      float Direction =  lir.GetDirection();
      bool Shoot =  lir.GetShoot();

      /*std::cout << 
       "Height " << Height << std::endl <<
       "Speed " << Speed << std::endl <<
       "Direction " << Direction << std::endl <<
       "Shoot " << Shoot << std::endl << std::endl;*/
       

      ////////// Update ship //////////
      
      cpt_laser++;
      
      // tir laser:
      if (Shoot and cpt_laser > 30)
      {

        Laser * laser = new Laser;
        laser->init();
        laser->setRenderableEntityID(renderable_lasers[0]->getID());
        laser->setLength(4.0f * GAME_SCALE);
        laser->setSpeed(GAME_SCALE);      
        laser->setPosition(ship->getPosition() + 2.0f*GAME_SCALE * ship->getMovingDirection());      
        laser->setMovingDirection(ship->getMovingDirection());
        lasers.push_back(laser); 
        cpt_laser = 0;  
        //printf("SHOOT!\n");
      }

      // Ship goes up
      if(Height > 0.0 and Height > 0.6) { 
        ship->setPosition(ship->getPosition() + Vector(0.0, 0.0, -15.0));
      // Ship goes down
      } else if(Height > 0.0 and Height < 0.4) { 
        ship->setPosition(ship->getPosition() + Vector(0.0, 0.0, 15.0));
      }

      if(Speed > 0.25) {
        //ship->Rotation X ? Y ?
        ship->accelerate(100*Speed);
      } else if(Speed < -0.15) {
        ship->accelerate(100*Speed);
      }

      if(Direction > 0.35) {
        ship->rotateZ(2, true);
      } else if(Direction < -0.35) {
        ship->rotateZ(-2, true);
      }

      ship->update(100.0f*1.0f/60.0f);

      // CRASHHHH au sol
      Point p = ship->getPosition();
      if(p.z > -2*GAME_SCALE) {      
        ship->setPosition(Point(p.x, p.y, -2*GAME_SCALE));
        ship->setSpeed(0.0);
      }
      // Arena limits:
      Point q = transform_PV(p);
      if (q.x < -0.96f or q.x > 0.96f or q.y < -0.96f or q.y > 0.96f or q.z < -0.96f or q.z  > 0.96f)
      {
        ship->setSpeed(0.0f);
      }
    } else {
        ship->setSpeed(0.0f);      
    }

      // Update lasers:
      updateLasers(lasers, transform_PV);
   

    
    ////////// Update asteroids //////////

    std::vector<std::list<Asteroid*>::iterator> clean;
    for (auto it=asteroids.begin(); it != asteroids.end(); ++it)
    {      
      (*it)->update(100.0f*1.0f/60.0f);

      Point p = (*it)->getPosition();
      if(p.z > 0.0) {      
        (*it)->setPosition(Point(p.x, p.y, 0.0));
        (*it)->setSpeed(0.0);
      }
      // Arena limits:
      Point q = transform_PV(p);
      if (q.x < -1.0f or q.x > 1.0f or q.y < -1.0f or q.y > 1.0f or q.z < -1.0f or q.z  > 1.0f) {
        clean.push_back(it);
      }
    }
    for (auto it: clean)
      asteroids.erase(it);

    // Test Collision asteroides:
    float shipRadius = renderable_ship.getBoundingRadius() * ship->getScale();
    Point shipPos = ship->getPosition();
    for (auto it=asteroids.begin(); it != asteroids.end(); ++it)
    {
      float asteroidRadius = (*it)->getBoundingRadius();
      Point p = (*it)->getPosition();
      float dist = distance(shipPos, p);
      if (dist < shipRadius + asteroidRadius)
      {
        sleep(5);
        shipCollide(&ship, renderable_ship.getID());
        asteroids.erase(it);
        break;
      }  
    }

    // Test explode asteroid:
    
    std::vector<std::list<Laser*>::iterator> cleanlist;
    for (auto laser_it = lasers.begin(); laser_it != lasers.end(); ++laser_it)
    {
      Point lasertip = (*laser_it)->getPosition() + (0.5f*(*laser_it)->getLength()) * (*laser_it)->getMovingDirection();      
      for (auto it = asteroids.begin(); it != asteroids.end(); ++it)
      {
        float asteroidRadius = (*it)->getBoundingRadius();
        Point asteroidPos = (*it)->getPosition(); 
        if (distance(asteroidPos, lasertip) < asteroidRadius * 0.8f) 
        {
          explodeAsteroid(it, asteroids, renderable_asteroids);   
          //std::cout << "EXPLODE " << std::endl;       
          cleanlist.push_back(laser_it);
          break;
        }
      }      
    }        
    for (auto it: cleanlist)
      lasers.erase(it);        
    

    ////////// Background Update //////////

    cam.get();
    GLuint background = cam.background();
    quad.setTexture(background);

    ////////// Clear Window //////////

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ////////// Background draw //////////

    GLuint prog;
    glDisable(GL_DEPTH_TEST);
    prog = background_program.getProgramID();
    glUseProgram(prog);
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, quad.getTexture());
    glUniform1i(glGetUniformLocation(prog, "image"), 0);
    quad.draw();
    glBindTexture(GL_TEXTURE_2D,0);
    glUseProgram(0);

    ////////// Renderable draw //////////

    glEnable(GL_DEPTH_TEST);
    prog = renderable_program.getProgramID();
    glUseProgram(prog);
    glUniformMatrix4fv(glGetUniformLocation(prog,"PV"), 1, GL_FALSE, &(PV[0][0]));
    glUniformMatrix4fv(glGetUniformLocation(prog,"Model"), 1, GL_TRUE, Identity().buffer());
    glUniform3f(glGetUniformLocation(prog,"lightPosition"), 40 * GAME_SCALE, 0 * GAME_SCALE, -40 * GAME_SCALE);
    glUniform3f(glGetUniformLocation(prog,"lightColor"), 1.0,1.0,1.0);
    glUniform3f(glGetUniformLocation(prog,"cameraPosition"), cam.position().x,cam.position().y,cam.position().z);
    
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, chess.getTexture());
    glUniform1i(glGetUniformLocation(prog, "tex"), 0);
    glUniform1i(glGetUniformLocation(prog, "use_texture"), 1);
    chess.draw();
    glBindTexture(GL_TEXTURE_2D, 0);
    glUniform1i(glGetUniformLocation(prog, "use_texture"), 0);

    glUniformMatrix4fv(glGetUniformLocation(prog, "Model"), 1, GL_TRUE, ship->getModelMatrix().buffer());
    renderable_ship.draw();
    
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
    glUseProgram(0);
    

    ////////// Lasers draw ////////////

    glUseProgram(laserprog);
    glUniformMatrix4fv(glGetUniformLocation(laserprog, "PV"), 1, GL_FALSE, &((cam.projection() * cam.view() * cam.gtoc())[0][0]));
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

    ////////// Update Window //////////

    win->refresh();

    ////////// Clean //////////

    cam.release();
  }

  background_program.destroy();
  renderable_program.destroy();
  quad.release();
  renderable_ship.release();

  return 0;
}

void explodeAsteroid(std::list<Asteroid*>::iterator & it, std::list<Asteroid*> & asteroids, const std::vector<RenderableAsteroid*> & renderable_asteroids)
{
  Asteroid * a = *it;
  asteroids.erase(it);

  if (a->getScale() < 0.1 * GAME_SCALE)
  {// si l'asteroide est trop petit inutile de le fragmenter
    delete a;
    return;
  }

  // Fragmentation:
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
    suba->setSpeed(getRandomFloat(0.0f, 0.08f) * GAME_SCALE);
    suba->setScale(a->getScale() / getRandomFloat(2.0f, 3.0f));

    asteroids.push_back(suba);
  }

  delete a;
}

void updateLasers(std::list<Laser*> & lasers, const Transform & PV)
{
  std::vector<std::list<Laser*>::iterator> cleanlist;

  for (std::list<Laser*>::iterator it = lasers.begin(); it != lasers.end(); ++it)
  {      
    (*it)->update(100.0f*1.0f/60.0f);
    Point p = (*it)->getPosition();
    Point q = PV(p);
    if (q.x < -1.0f or q.x > 1.0f or q.y < -1.0f or q.y > 1.0f or q.z < -1.0f or q.z  > 1.0f)
      cleanlist.push_back(it);    
  }

  //if (!cleanlist.empty())
  //  printf("Removed %d lasers !\n", (int)cleanlist.size());
  for (auto it: cleanlist)
    lasers.erase(it);
}

void shipCollide(GameEntity ** ship, uint64 id)
{

  *ship = new GameEntity;
  (*ship)->init();
  (*ship)->setRenderableEntityID(id);
  (*ship)->setPosition(Point(6.0 * GAME_SCALE,4.0 * GAME_SCALE,-2*GAME_SCALE));// position de départ du vaisseau
  (*ship)->rotateX(90.0f, false);// orientation de départ
  (*ship)->setMovingDirection(Vector(0.0f, -1.0f, 0.0f));
  (*ship)->setScale(GAME_SCALE);
}