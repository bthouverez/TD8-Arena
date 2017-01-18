#include <iostream>

#include "leapinputreader.h"
#include "cameraarena.hpp"
#include "gldisplay.hpp"

int main(int argc, char** argv) {
  // Create LeapInputReader
  //LeapInputReader reader;
  CameraArena cam = CameraArena(1920, 1080, 60, 1);
  //cam.intrinsics(8,5,196,50);
  cam.read("data/intrinsics.txt");
  std::cout << "Press c " << std::endl;

  GLDisplay display(1920, 1080, 8, 5, 196, "data/tie.off");
  display.loadIntrinsics(cam.amatrix(), cam.kmatrix());

  // Keep this process running until Enter is pressed
  while(true)
  {
      cam.get();
      //cam.show(); 
      cam.extrinsics(8,5,196);
      cv::Mat back = cam.mat();
      display.updateBackground(back);
      display.calcTransformation(cam.rmatrix(), cam.tmatrix());
      display.drawGLDisplay();

      if(cv::waitKey(1) == 'q') break;
  }

  return 0;
}
