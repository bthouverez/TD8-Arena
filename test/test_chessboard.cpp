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
  
  ////////// Camera //////////

  CameraArena cam = CameraArena(width, height, rate, device);
  // Load intrinsics parameters
  cam.read(argv[7]);
  // Calc extrinsics parameters
  cam.get();
  cam.extrinsics(chess_width, chess_height, chess_size);
  cam.frustum( width, height );

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

  while(win->isActive())
  { 

    ////////// Background Update //////////

    cam.get();
    cam.extrinsics(chess_width, chess_height, chess_size);
    
    glm::mat4 PV = cam.projection() * cam.view() * cam.gtoc();
    Transform transform_PV(
      PV[0][0], PV[1][0], PV[2][0], PV[3][0],
      PV[0][1], PV[1][1], PV[2][1], PV[3][1],
      PV[0][2], PV[1][2], PV[2][2], PV[3][2],
      PV[0][3], PV[1][3], PV[2][3], PV[3][3]
    );

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
    glBindTexture(GL_TEXTURE_2D, 0);
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
    glUniform1i(glGetUniformLocation(prog,"use_texture"), 1);
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, chess.getTexture());
    glUniform1i(glGetUniformLocation(prog, "tex"), 0);
    chess.draw();
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
    
    ////////// Update Window //////////

    win->refresh();

    ////////// Clean //////////

    cam.release();
  }

  background_program.destroy();
  renderable_program.destroy();
  quad.release();
  chess.release();

  return 0;
}