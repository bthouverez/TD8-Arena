#include <iostream>

#include "leapinputreader.h"
#include "cameraarena.hpp"

int main(int argc, char** argv) {
  // Create LeapInputReader
  //LeapInputReader reader;
  CameraArena cam = CameraArena(640, 480, 30);

  cam.intrinsics(5,4,25,30);

  // Keep this process running until Enter is pressed
  while(true)
  {
      cam.get();
      cam.show(); 
      if(cv::waitKey(1) == 'q') break;
  }

  return 0;
}
