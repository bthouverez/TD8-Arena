#ifndef WINDOW_HEADER
#define WINDOW_HEADER

#include "common.hpp" // GL

typedef GLFWwindow NATIVEWINDOW;


//FrwdDcl:
class Window;

static void glfw_key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    (void) scancode;
    (void) mods;
    if (key == GLFW_KEY_Q && action == GLFW_PRESS)
        glfwSetWindowShouldClose(Window::getNativeWindow(), GL_TRUE);
}


//! SINGLETON
class Window
{
public:

  static void init_fullscreen()
  {
    if (!initdone)
    {
      m_instance = new Window(true);
      m_instance->init();
      initdone = true;
    }
  }

  static void init(int width, int height)
  {
    if (!initdone)
    {
      m_instance = new Window(width, height);
      m_instance->init();
      initdone = true;
    }
  }

  //! Swap les buffers et raffraîchit l'écran pour une nouvelle frame.
  static void refresh() 
  {
    glfwSwapBuffers(m_instance->m_window);
    glfwPollEvents();   // Garde la fenetre active
  }

  static void release()
  {
    m_instance->release();
  }

  static NATIVEWINDOW* getNativeWindow() { return m_instance->m_window; }

  static void getSize(int & width, int & height) { width = m_instance->m_width; height = m_instance->m_height; }

  static Window* getInstance() { return m_instance; }


private:
  explicit Window(bool fullscreen = false) : m_fullscreen(fullscreen) {}

  Window(int width, int height) : m_width(width), m_height(height) {}

  Window * m_instance = nullptr;


  void init()
  {
    const char *title = "TD8-ARENA";
    // create window and OpenGL context
    if (!glfwInit()) {
        fprintf(stderr, "GLFW3: failed to initialize\n");
        exit(EXIT_FAILURE);
    }
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
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

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1);

    // initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }  

    // keyboard listening:
    glfwSetKeyCallback(m_window, glfw_key_callback);
  }  

  void release()
  {
    glfwDestroyWindow(m_window);
    glfwTerminate();  
  }  

  


  GLFWwindow *m_window;
  int m_width = 1600, m_height = 900;
  bool m_fullscreen = false;
};

#endif
