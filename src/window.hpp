#ifndef WINDOW_HEADER
#define WINDOW_HEADER

#include "common.hpp" // GL
#include <stdio.h>
#include <stdlib.h>

typedef GLFWwindow NATIVEWINDOW;



static void glfw_error_callback(int err, const char * description)
{
  fprintf(stderr, "GLFW error %d = %s\n", err, description);
}

//FrwdDcl:
static void glfw_key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);



//! SINGLETON
class Window
{
public:

  static Window* init_fullscreen()
  {
    if (!initdone)
    {
      m_instance = new Window(true);      
      initdone = true;
    }
    return m_instance;
  }

  static Window* init(int width, int height)
  {
    if (!initdone)
    {
      m_instance = new Window(width, height);      
      initdone = true;
    }
    return m_instance;
  }

  static Window* getInstance() { return m_instance; }


  bool isActive() 
  {
    return not glfwWindowShouldClose(m_window);
  }

  NATIVEWINDOW* getNativeWindow() { return m_window; }
  
  void getSize(int & width, int & height) { width = m_width; height = m_height; }
  
  //! Swap les buffers et raffraîchit l'écran pour une nouvelle frame.
  void refresh() 
  {
    glfwSwapBuffers(m_instance->m_window);
    glfwPollEvents();   // Garde la fenetre active
  }
  
  void release()
  {
    glfwDestroyWindow(m_window);
    glfwTerminate();  
  }  

  
private:
  
  static bool initdone;
  static Window* m_instance;

  explicit Window(bool fullscreen = true) : m_fullscreen(fullscreen)
  {
    init_internal();
  }

  Window(int width, int height) : m_width(width), m_height(height) 
  {
    init_internal();
  }

  void init_internal()
  {
    const char *title = "TD8-ARENA";
    // create window and OpenGL context
    if (!glfwInit()) {
        fprintf(stderr, "GLFW3: failed to initialize\n");
        exit(EXIT_FAILURE);
    }
    printf("Compiled against GLFW %i.%i.%i\n",
       GLFW_VERSION_MAJOR,
       GLFW_VERSION_MINOR,
       GLFW_VERSION_REVISION);
    int major, minor, revision;
    glfwGetVersion(&major, &minor, &revision);
    printf("Running against GLFW %i.%i.%i\n", major, minor, revision);

    glfwSetErrorCallback(glfw_error_callback);

    // WINDOW CREATION:
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    if (m_fullscreen)
    { 
      GLFWmonitor* monitor = glfwGetPrimaryMonitor();
      const GLFWvidmode* mode = glfwGetVideoMode(monitor);
      glfwWindowHint(GLFW_RED_BITS, mode->redBits);
      glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
      glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
      glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
      m_window = glfwCreateWindow(mode->width, mode->height, title, monitor, NULL);
      m_width = mode->width;
      m_height = mode->height;
    }
    else {
      m_window = glfwCreateWindow(m_width, m_height, title, NULL, NULL);  
    }
    if (m_window == NULL)
    {
      fprintf(stderr, "GLFW failed to create window.\n");
      exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1);

    // initialize GLEW
    glewExperimental = true; // Needed for core profile
    GLenum status = glewInit();
    if (status != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW -- %s\n", glewGetErrorString(status));
        exit(EXIT_FAILURE);
    }      
    while(glGetError() != GL_NO_ERROR) {;}// purge les erreurs opengl generees par glew 

    // keyboard listening:
    glfwSetKeyCallback(m_window, glfw_key_callback);
  }  


  GLFWwindow *m_window;
  int m_width = 1600, m_height = 900;
  bool m_fullscreen;
};


static void glfw_key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    (void) scancode;
    (void) mods;
    if (key == GLFW_KEY_Q && action == GLFW_PRESS)
        glfwSetWindowShouldClose(Window::getInstance()->getNativeWindow(), GL_TRUE);
}


#endif
