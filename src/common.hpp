#ifndef COMMON_HEADER
#define COMMON_HEADER

// --- GL INCLUDES ---
// include from OpenGL, glm, glfw
#define GLFW_INCLUDE_NONE
#include <GL/glew.h>    
#include <GLFW/glfw3.h>
#include "opencv2/opencv.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

//! Base Class (uuid)
class Common
{
public: 

  Common() : id(Common::new_id()) {}
  virtual ~Common(){}

  uint64 getID() const { return id; }  



private:

  static uint64 new_id(){ return ++id_compteur; }
  static uint64 id_compteur;

  uint64 id;
};
#endif
