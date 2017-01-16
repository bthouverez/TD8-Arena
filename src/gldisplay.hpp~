#ifndef GLDISPLAY_H
#define GLDISPLAY_H

// include from standard c++ lib
#include <iostream>
#include <fstream>
#include <fstream>
#include <vector>

// include from OpenCV
#include <opencv2/opencv.hpp>

// include from OpenGL, glm, glfw
#define GLFW_INCLUDE_NONE
#include <GL/glew.h>
#include <GL/glut.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define ATTRIB_POINT 0
#define ATTRIB_COLOR 1
#define ATTRIB_TEXUV 1

// Bastien Thouverez 
// Basile Fraboni 
// 2017 - 01

//-------------------------------------------------------------------------
//    OpenGL display
//-------------------------------------------------------------------------

struct graphics_context {
    GLFWwindow *window;
    GLuint program;
    GLuint program_tex;
    GLuint axis_vertex_buffer;
    GLuint chessboard_vertex_buffer;
    GLuint texture_vertex_buffer;
    GLuint axis_color_buffer;
    GLuint chessboard_color_buffer;
    GLuint texture_coord_buffer;
    GLuint off_vertex_buffer;
    GLuint off_color_buffer;
    GLuint off_vao;
    GLuint axis_vao;
    GLuint chessboard_vao;
    GLuint texture_vao;
    GLuint texture;
};

class GLDisplay {

public:

	// w : window width
	// h : window height
	// pw : chessboard width
	// ph : chessboard height
	// size : chessboard square size
	// file : object file to load .off 
    GLDisplay(int w, int h, int pw, int ph, float size, std::string file);

    ~GLDisplay();

    // Load intrinsic parameters
    void loadIntrinsics(const cv::Mat & A, const cv::Mat & K);
    // Update background texture
    void updateBackground(cv::Mat &);
    // Calc Global to Camera matrix (gtoc)
    void calcTransformation(const cv::Mat & R, const cv::Mat & T);
    // Draw all gl components on the OpenGL window
    void drawGLDisplay();

protected:

	// Init / clean
    int initGLDisplay();
    int cleanGLDisplay();
    // Draw components
    void drawChessboard();
    void drawAxis();
    void drawBackground();
    void drawOff();
    // Object loading
    void loadOff(std::string filename);
    // Background process
    void makeBackground(cv::Mat & mat, GLenum minFilter, GLenum magFilter, GLenum wrapFilter); // credits R3dux 
    void releaseBackground();
    // Frustum
    void calcFrustum();
    // Point projection
    glm::vec3 unproject(glm::vec2 point, float sz);

    struct graphics_context context;		// OpenGL context

    int Window_Width;					
    int Window_Height;

    int patternWidth = 0;					// Chessboard width
    int patternHeight = 0;					// Chessboard height
    float squareSize = 0;					// Chessboard square size

    glm::mat3 intrinsics = glm::mat3(1.0f);		// Intrinsics parameters
    glm::vec4 distorsions =  glm::vec4(1.0f);	// Distorsions matrix
    glm::mat4 gtoc = glm::mat4(1.0f);			// World to camera transformation

    GLuint backtexture;				// Background OpenGL texture

    bool object = false;			// True if object to draw    
	std::vector<glm::vec3> points;	// Object vertex
	std::vector<glm::vec2> edges;	// Object edges

    float near;
    float far;
    float * frustum;

    // Shaders
 	const GLchar *vert_shader =
    "#version 330\n"
    "layout(location = 0) in vec3 position;\n"
    "layout(location = 1) in vec3 color;\n"
    "out vec3 vcolor;\n"
    "uniform mat4 MVP;\n"
    "\n"
    "void main()\n"
    "{\n"
    "    gl_Position = MVP * vec4(position, 1.0);\n"
    "    vcolor = color;\n"
    "}\n";

 	const GLchar *frag_shader =
    "#version 330\n"
    "in vec3 vcolor;\n"
    "out vec4 color;\n"
    "\n"
    "void main() {\n"
    "    color = vec4(vcolor, 0);\n"
    "}\n";


 	const GLchar *vert_shader_tex =
    "#version 330 core\n"
    "layout(location = 0) in vec3 position;\n"
    "layout(location = 1) in vec2 vertexUV;\n"
    "out vec2 UV;\n"
    "\n"
    "uniform mat4 MVP;\n"
    "\n"
    "void main(){\n"
    "    gl_Position =  MVP * vec4(position, 1.0);\n"
    "    UV = vertexUV;\n"
    "}\n";

 	const GLchar *frag_shader_tex =
    "#version 330 core\n"
    "in vec2 UV;\n"
    "out vec3 color;\n"
    "\n"
    "uniform sampler2D myTextureSampler;\n"
    "\n"
    "void main(){\n"
    "    color = texture( myTextureSampler, UV ).rgb;\n"
    "}\n";
};

#endif