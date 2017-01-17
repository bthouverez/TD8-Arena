#include <iostream>

#include "leapinputreader.h"
#include "cameraarena.hpp"

int main(int argc, char** argv) {
  // Create LeapInputReader
  //LeapInputReader reader;
  CameraArena cam = CameraArena(1920, 1080, 30);

  cam.intrinsics(6,5,25.5,20);

  // Keep this process running until Enter is pressed
  while(true)
  {
      cam.get();
      cam.show(); 
      if(cv::waitKey(1) == 'q') break;
  }

  return 0;
}
