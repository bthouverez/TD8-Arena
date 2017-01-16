#ifndef COMMON_HEADER
#define COMMON_HEADER

// --- GL INCLUDES ---
// include from OpenGL, glm, glfw
#define GLFW_INCLUDE_NONE
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


typedef unsigned long long uint64;



//! Base Class (uuid)
class Common
{
public: 
  Common() : id(Common::new_id()) {}

  uint64 getID() { return id; }  

private:
  static uint64 new_id(){ return ++id_compteur; }

  static uint64 id_compteur;
};
#endif
