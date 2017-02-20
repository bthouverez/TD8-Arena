#include "common.hpp"
#include "window.hpp"
#include "shader.hpp"
#include "cameraarena.hpp"
#include "screenquad.hpp"
#include "leapinputreader.h"
#include "game_entity.hpp"
#include "renderable_entity.hpp"

#include <string>

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

  ////////// GL Window //////////

  Window * win = Window::init(width, height);

  ////////// Leap Joystick //////////

  LeapInputReader lir;
  
  ////////// Camera //////////

  CameraArena cam = CameraArena(width, height, rate, device);
  // Load intrinsics parameters
  cam.read(argv[7]);
  // Calc extrinsics parameters
  cam.get();
  while(!cam.extrinsics(chess_width, chess_height, chess_size))
  {
    std::cout << "Camera calibration ..." << std::endl;
    cam.get();
  }
  cam.frustum( width, height );

  ////////// Ship //////////

  RenderableEntity renderable_ship;
  renderable_ship.loadOBJ("data/object/tie.obj");
  GameEntity ship;
  ship.setRenderableEntityID(renderable_ship.getID());

  ////////// GL Parameters //////////

  //glEnable(GL_DEPTH_TEST);
  glDisable(GL_DEPTH_TEST);
  glClearDepth(1.0f);
  glClearColor(0, 0, 0, 1);

  ////////// Background init //////////

  ScreenQuad quad;
  quad.init();

  ////////// Shaders init //////////

  Shader background_program("data/shader/screenquad.vertex.glsl", "data/shader/screenquad.fragment.glsl");
  Shader renderable_program("data/shader/default_mesh.vertex.glsl", "data/shader/default_mesh.fragment.glsl");
  if (!background_program.init() || !renderable_program.init())
  {
    std::cout << "Can not load shader programs ..." << std::endl;
    exit(1);
  }


  while(win->isActive())
  {
    ////////// Joystick values //////////

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

    ////////// Update ship position //////////

    // TODO

    ////////// Background Update //////////

    cam.get();
    GLuint background = cam.background();
    quad.setTexture(background);

    ////////// Calc transformation matrix //////////



    ////////// Clear Window //////////

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ////////// Background draw //////////

    GLuint prog = background_program.getProgramID();
    glUseProgram(prog);
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, quad.getTexture());
    glUniform1i(glGetUniformLocation(prog, "image"), 0);
    quad.draw();
    glUseProgram(0);

    ////////// Renderable draw //////////


    float scale = 8.*chess_size;

    Transform tr =  Translation(3.f*chess_size,1.5f*chess_size,-1.f*chess_size) * 
                    Rotation(Vector(1.f, 0.f, 0.f), -90.f) * 
                    Scale(scale,scale,scale);

    prog = renderable_program.getProgramID();
    glUseProgram(prog);
    glUniformMatrix4fv(glGetUniformLocation(prog,"PV"), 1, GL_TRUE, (cam.projection() * cam.view()).buffer());
    glUniformMatrix4fv(glGetUniformLocation(prog,"Model"), 1, GL_TRUE, (cam.gtoc() * tr).buffer());
    glUniform3f(glGetUniformLocation(prog,"lightPosition"), 200.,200.,200.);
    glUniform3f(glGetUniformLocation(prog,"lightColor"), 1.,1.,0.);
    glUniform3f(glGetUniformLocation(prog,"cameraPosition"), cam.position().x,cam.position().y,cam.position().z);
    renderable_ship.draw();
    glUseProgram(0);

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
