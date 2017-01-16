#include <iostream>
#include <opencv2/opencv.hpp>
#include "leapinputreader.h"

int main(int argc, char** argv) {
  // Create LeapInputReader
  LeapInputReader reader;

  // Keep this process running until Enter is pressed
  while(true)
  {
      reader.UpdateHandLocations();
      
      if(reader.IsValidInputLastFrame())
      {
          std::cout << " Frame OK" << std::endl;
      }
      else
      {
          std::cout << " Not readable frame" << std:: endl;
      }

      
  }

  return 0;
}
