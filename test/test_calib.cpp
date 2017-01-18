#include <iostream>
#include "cameraarena.hpp"
#include "gldisplay.hpp"

int main(int argc, char** argv) {
  
  if(argc < 9 || argc > 10)
  {
    std::cerr << "Usage : bin/calib <width> <height> <rate>";
    std::cerr << " <chessboard_width> <chessboard_height> <square_size> <nb_frame> <camera_parameters_file> [<device>]" << std::endl;
    exit(1);
  }

  int width = atoi(argv[1]);
  int height = atoi(argv[2]);
  float rate = atof(argv[3]);
  int chess_width = atoi(argv[4]);
  int chess_height = atoi(argv[5]);
  int chess_size = atoi(argv[6]);
  int nb = atoi(argv[7]);
  int device = argc == 10 ? atoi(argv[9]) : 0;

  CameraArena cam = CameraArena(width, height, rate, device);
  cam.intrinsics(chess_width, chess_height, chess_size, nb);
  cam.write(argv[8]);

  while(true)
  {
      if(cv::waitKey(1) == 'q') break;
  }

  return 0;
}
