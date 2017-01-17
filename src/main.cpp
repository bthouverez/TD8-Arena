#include <iostream>

#include "leapinputreader.h"
#include "cameraarena.hpp"
#include "gldisplay.hpp"

int main(int argc, char** argv) {
  // Create LeapInputReader
  //LeapInputReader reader;
  CameraArena cam = CameraArena(1920, 1080, 30);
  cam.intrinsics(6,5,25.5,30);

  GLDisplay display(1920, 1080, 6, 5, 25.5, "data/tie.off");
  display.loadIntrinsics(cam.amatrix(), cam.kmatrix());

  // Keep this process running until Enter is pressed
  while(true)
  {
      cam.get();
      cam.extrinsics(6,5,25.5); 
      //cam.show(); 
      cv::Mat back = cam.mat();
      display.updateBackground(back);
      display.calcTransformation(cam.rmatrix(), cam.tmatrix());
      display.drawGLDisplay();

      if(cv::waitKey(1) == 'q') break;
  }

  return 0;
}
