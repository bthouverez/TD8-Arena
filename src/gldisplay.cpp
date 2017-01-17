#include "gldisplay.hpp"

// Bastien Thouverez
// Basile Fraboni 
// 2017 - 01

static GLuint compile_shader(GLenum type, const GLchar *source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    GLint param;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &param);
    if (!param) {
        GLchar log[4096];
        glGetShaderInfoLog(shader, sizeof(log), NULL, log);
        fprintf(stderr, "error: %s: %s\n",
                type == GL_FRAGMENT_SHADER ? "frag" : "vert", (char *) log);
        exit(EXIT_FAILURE);
    }
    return shader;
}

static GLuint link_program(GLuint vert, GLuint frag)
{
    GLuint program = glCreateProgram();
    glAttachShader(program, vert);
    glAttachShader(program, frag);
    glLinkProgram(program);
    GLint param;
    glGetProgramiv(program, GL_LINK_STATUS, &param);
    if (!param) {
        GLchar log[4096];
        glGetProgramInfoLog(program, sizeof(log), NULL, log);
        fprintf(stderr, "error: link: %s\n", (char *) log);
        exit(EXIT_FAILURE);
    }
    return program;
}

GLDisplay::GLDisplay(int w, int h, int pw, int ph, float size, std::string file) :   
															Window_Width(w),
                                                            Window_Height(h),
                                                            patternWidth(pw),
                                                            patternHeight(ph),
                                                            squareSize(size)
                                                            {
                                                            	// Init window & OpenGL context
                                                            	initGLDisplay();
    															// Load off file
                                                            	if(!file.empty())
                                                            	{	
                                                            		object = true;
    																loadOff(file);
    															}
                                                            }
GLDisplay::~GLDisplay(){cleanGLDisplay();}


void GLDisplay::loadIntrinsics(const cv::Mat & A, const cv::Mat & K)
{
    distorsions = glm::vec4(K.at<double>(0), K.at<double>(1), K.at<double>(2), K.at<double>(3));

    intrinsics = glm::mat3(  A.at<double>(0,0), A.at<double>(1,0), A.at<double>(2,0),
                             A.at<double>(0,1), A.at<double>(1,1), A.at<double>(2,1),
                             A.at<double>(0,2), A.at<double>(1,2), A.at<double>(2,2));
}

void GLDisplay::drawBackground()
{

    static std::vector<float> VERTICES;
    static std::vector<float> TEXCOORDS;

    glm::vec4 p1 =   	glm::vec4(unproject(glm::vec2(0.f,0.f),far-10.f), 1.0);
    glm::vec4 p2 =   	glm::vec4(unproject(glm::vec2((float)Window_Width+1.f,0.f),far-10.f), 1.0);
    glm::vec4 p3 =   	glm::vec4(unproject(glm::vec2(0.f,(float)Window_Height+1.f),far-10.f), 1.0);
    glm::vec4 p4 =   	glm::vec4(unproject(glm::vec2((float)Window_Width+1.f,(float)Window_Height+1.f),far-10.f), 1.0);

    VERTICES.push_back(p1[0]);
    VERTICES.push_back(p1[1]);
    VERTICES.push_back(p1[2]);
    VERTICES.push_back(p2[0]);
    VERTICES.push_back(p2[1]);
    VERTICES.push_back(p2[2]);
    VERTICES.push_back(p3[0]);
    VERTICES.push_back(p3[1]);
    VERTICES.push_back(p3[2]);
    VERTICES.push_back(p4[0]);
    VERTICES.push_back(p4[1]);
    VERTICES.push_back(p4[2]);

    std::vector<float> t1({0.0,0.0});
    std::vector<float> t2({1.0,0.0});
    std::vector<float> t3({0.0,1.0});
    std::vector<float> t4({1.0,1.0});

    TEXCOORDS.insert(TEXCOORDS.end(),t1.begin(),t1.end());
    TEXCOORDS.insert(TEXCOORDS.end(),t2.begin(),t2.end());
    TEXCOORDS.insert(TEXCOORDS.end(),t3.begin(),t3.end());
    TEXCOORDS.insert(TEXCOORDS.end(),t4.begin(),t4.end());

    // Bind texture
    glBindTexture(GL_TEXTURE_2D, backtexture);
    glBindVertexArray(context.texture_vao);
    // vertices
    glBindBuffer(GL_ARRAY_BUFFER, context.texture_vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, VERTICES.size()*sizeof(VERTICES[0]), VERTICES.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(ATTRIB_POINT /*shader layout*/, 3 /*size*/, GL_FLOAT /*type*/, GL_FALSE /*normalized?*/, 0 /*stride*/, 0 /*array buffer offset*/);
    glEnableVertexAttribArray(ATTRIB_POINT);
    // texcoord
    glBindBuffer(GL_ARRAY_BUFFER, context.texture_coord_buffer);
    glBufferData(GL_ARRAY_BUFFER, TEXCOORDS.size()*sizeof(TEXCOORDS[0]), TEXCOORDS.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(ATTRIB_COLOR /*shader layout*/, 2 /*size*/, GL_FLOAT /*type*/, GL_FALSE /*normalized?*/, 0 /*stride*/, 0 /*array buffer offset*/);
    glEnableVertexAttribArray(ATTRIB_COLOR);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // draw
    glBindVertexArray(context.texture_vao);
    glDrawArrays(GL_TRIANGLES, 0, VERTICES.size());
    glBindVertexArray(0);

    // Free the texture memory
    releaseBackground();
}

void GLDisplay::updateBackground(cv::Mat & back)
{
    // Make background OpenGL texture from cv::Mat
    makeBackground(back, GL_NEAREST, GL_NEAREST, GL_CLAMP);
}

void GLDisplay::releaseBackground()
{
    glDeleteTextures(1, &backtexture);
}

void GLDisplay::makeBackground(cv::Mat & mat, GLenum minFilter, GLenum magFilter, GLenum wrapFilter)
{
    // Credits 
    // Function from R3dux 
    // http://r3dux.org/2012/01/how-to-convert-an-opencv-cvmat-to-an-opengl-texture/

    // Generate a number for our textureID's unique handle
    GLuint textureID;
    glGenTextures(1, &textureID);
    // Bind to our texture handle
    glBindTexture(GL_TEXTURE_2D, textureID);
    // Catch silly-mistake texture interpolation method for magnification
    if (magFilter == GL_LINEAR_MIPMAP_LINEAR  ||
        magFilter == GL_LINEAR_MIPMAP_NEAREST ||
        magFilter == GL_NEAREST_MIPMAP_LINEAR ||
        magFilter == GL_NEAREST_MIPMAP_NEAREST)
    {
        magFilter = GL_LINEAR;
    }
    // Set texture interpolation methods for minification and magnification
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
    // Set texture clamping method
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapFilter);
    // Set incoming texture format to:
    // GL_BGR       for CV_CAP_OPENNI_BGR_IMAGE,
    // GL_LUMINANCE for CV_CAP_OPENNI_DISPARITY_MAP,
    // Work out other mappings as required ( there's a list in comments in main() )
    GLenum inputColourFormat = GL_BGR;
    if (mat.channels() == 1)
    {
        inputColourFormat = GL_LUMINANCE;
    }
    // Create the texture
    glTexImage2D(GL_TEXTURE_2D,     // Type of texture
                 0,                 // Pyramid level (for mip-mapping) - 0 is the top level
                 GL_RGB,            // Internal colour format to convert to
                 mat.cols,          // Image width  i.e. 640 for Kinect in standard mode
                 mat.rows,          // Image height i.e. 480 for Kinect in standard mode
                 0,                 // Border width in pixels (can either be 1 or 0)
                 inputColourFormat, // Input image format (i.e. GL_RGB, GL_RGBA, GL_BGR etc.)
                 GL_UNSIGNED_BYTE,  // Image data type
                 mat.ptr());        // The actual image data itself
    // If we're using mipmaps then generate them. Note: This requires OpenGL 3.0 or higher
    if (minFilter == GL_LINEAR_MIPMAP_LINEAR  ||
        minFilter == GL_LINEAR_MIPMAP_NEAREST ||
        minFilter == GL_NEAREST_MIPMAP_LINEAR ||
        minFilter == GL_NEAREST_MIPMAP_NEAREST)
    {
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    // Keep GL index of texture
    backtexture = textureID;
}

void GLDisplay::loadOff(std::string filename) {

    std::ifstream file( filename.c_str() );
    if( !file ) 
    {
        std::cout << "can not load file " << std::endl;
    }
    else
    {
        // Standard File OFF readable by this function:

        // OFF
        // nb_vertex nb_faces nb_normals
        // x y z 								# vertex 
        // ...
        // a b c 								# faces 
        // ...
        // nx ny nz 							# normals  
        // ...

        // Read header
        std::string line;
        file >> line;
        unsigned int vertex,faces_nb,normal;
        file >> vertex >> faces_nb >> normal;
        

        // Read vertex
        std::vector<float> bounds({1000,1000,1000,-1000,-1000,-1000});
        for(unsigned int i = 0; i < vertex ; i++)
        {
            glm::vec3 point;
            file >> point[0] >> point[1] >> point[2];
            points.push_back(point);
            // Min bound
            bounds[0] = point[0] < bounds[0] ? point[0] : bounds[0];
            bounds[1] = point[1] < bounds[1] ? point[1] : bounds[1];
            bounds[2] = point[2] < bounds[2] ? point[2] : bounds[2];
            // Max Bound
            bounds[3] = point[0] > bounds[3] ? point[0] : bounds[3];
            bounds[4] = point[1] > bounds[4] ? point[1] : bounds[4];
            bounds[5] = point[2] > bounds[4] ? point[2] : bounds[5];
        }

        // Transform and scale object bounding base square 
        glm::vec2 cmin = glm::vec2(bounds[0],bounds[1]);
        glm::vec2 cmax = glm::vec2(bounds[3],bounds[4]);
        float d = glm::distance(cmin, cmax);
        float scale = 10*squareSize/d;

        for(unsigned int i = 0; i < points.size() ; i++)
        {
        	// Add transformation to show the model on the chessboard
        	glm::mat4 tr = glm::mat4(1.0);
        	tr = glm::translate(tr, glm::vec3(3.f*squareSize,1.5f*squareSize,-5.f*squareSize));
        	tr = glm::rotate(tr, -90.f, glm::vec3(1.f, 0.f, 0.f));
        	// Add scale
        	tr = glm::scale(tr, glm::vec3(scale,scale,scale));
        	// Transformed point
        	glm::vec4 tmp = glm::vec4(points[i],1.0); 
            tmp = tr * tmp;
            tmp = tmp/tmp[3];
            points[i] = glm::vec3(tmp[0],tmp[1],tmp[2]);
        }

        // Read faces
        for(unsigned int i = 0; i < faces_nb ; i++)
        {
        	// Get vertex index
            int a, b, c, nb;
            file >> nb >> a >> b >> c;
            // Wireframe display mode
            glm::vec2 edge1 = glm::vec2(a,b);
            glm::vec2 edge2 = glm::vec2(b,c);
            glm::vec2 edge3 = glm::vec2(c,a);
            edges.push_back(edge1);
            edges.push_back(edge2);
            edges.push_back(edge3);
        }
        // Close file
        file.close();
        // Nb vertex in model
        std::cout << points.size() << std::endl;
    }
}

void GLDisplay::drawOff() 
{
    static std::vector<float> VERTICES;	// Object vertex
    static std::vector<float> COLORS;	// Object colors

    if(VERTICES.empty())
	{ 
		// Done 1st time only
	    // GL Vertex & color buffers 
        std::vector<float> red   = { 1.0f, 0.0f, 0.0f };

        for(auto edge : edges)
        {
            int a , b;
            a = edge[0];
            b = edge[1];

            VERTICES.push_back(points[a][0]);
            VERTICES.push_back(points[a][1]);
            VERTICES.push_back(points[a][2]);
            VERTICES.push_back(points[b][0]);
            VERTICES.push_back(points[b][1]);
            VERTICES.push_back(points[b][2]);

            for(int i = 0; i < 6; i++)
                COLORS.insert(COLORS.end(),red.begin(),red.end());
        }

        glBindVertexArray(context.off_vao);
        // vertices
        glBindBuffer(GL_ARRAY_BUFFER, context.off_vertex_buffer);
        glBufferData(GL_ARRAY_BUFFER, VERTICES.size()*sizeof(VERTICES[0]), VERTICES.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(ATTRIB_POINT /*shader layout*/, 3 /*size*/, GL_FLOAT /*type*/, GL_FALSE /*normalized?*/, 0 /*stride*/, 0 /*array buffer offset*/);
        glEnableVertexAttribArray(ATTRIB_POINT);
        // color
        glBindBuffer(GL_ARRAY_BUFFER, context.off_color_buffer);
        glBufferData(GL_ARRAY_BUFFER, COLORS.size()*sizeof(COLORS[0]), COLORS.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(ATTRIB_COLOR /*shader layout*/, 3 /*size*/, GL_FLOAT /*type*/, GL_FALSE /*normalized?*/, 0 /*stride*/, 0 /*array buffer offset*/);
        glEnableVertexAttribArray(ATTRIB_COLOR);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    // Wireframe display mode
    glBindVertexArray(context.off_vao);
	glDrawArrays(GL_LINES, 0, VERTICES.size());
    glBindVertexArray(0);
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    (void) scancode;
    (void) mods;
    if (key == GLFW_KEY_Q && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

void GLDisplay::drawGLDisplay()
{
    // OpenGL display
    glClearColor(0.15, 0.15, 0.15, 1);
    glClearDepth(1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Frustum Projection
    calcFrustum();
    glm::mat4 Projection = glm::frustum(frustum[0], frustum[1], frustum[2], frustum[3], frustum[4], frustum[5]);
    // View matrix
    glm::mat4 View = glm::lookAt(glm::vec3(0.f,0.f,0.f),glm::vec3(0.f,0.f,1.f),glm::vec3(0.f,-1.f,0.f));
    // Model matrix
    glm::mat4 Model = glm::transpose(gtoc);
    // ModelViewProjection : multiplication of our 3 matrices
    glm::mat4 MVP = Projection * View * Model;
    // ModelView : background texture projection matrix
    glm::mat4 MVPc = Projection * View * glm::mat4(1.0);

    // Draw texture background
    glUseProgram(context.program_tex);
    GLuint MatrixID = glGetUniformLocation(context.program_tex, "MVP");
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVPc[0][0]);
    drawBackground();
    glUseProgram(0);
    // Draw gl components
    glUseProgram(context.program);
    MatrixID = glGetUniformLocation(context.program, "MVP");
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
    drawChessboard();
    drawAxis();
    // Draw object if set
    if(object) drawOff();

    // swap buffers
    glUseProgram(0);
    glfwSwapBuffers(context.window);
    glfwPollEvents();   // Garde la fenetre active

}

glm::vec3 GLDisplay::unproject(glm::vec2 point, float sz)
{
    return glm::vec3(glm::inverse(intrinsics) * glm::vec3(point,1)) * sz;
}

void GLDisplay::calcFrustum()
{
    // Camera depth vision
    near = 20.f; // 2 cm  
    far = 2000.f; // 2 meters
    // Frustum faces points
    glm::vec3 mid =   	unproject(glm::vec2(Window_Width/2.f, Window_Height/2.f),near);
    glm::vec3 left =   	unproject(glm::vec2(0.f, Window_Height/2.f),near);
    glm::vec3 right =   unproject(glm::vec2(Window_Width, Window_Height/2.f),near);
    glm::vec3 bottom =  unproject(glm::vec2(Window_Width/2.f, 0.f),near);
    glm::vec3 top =   	unproject(glm::vec2(Window_Width/2.f, Window_Height),near);
    // Frustum
    frustum[0] = -1.0f * distance(mid, left);
    frustum[1] = 1.0 * distance(mid, right);
    frustum[2] = -1.0f * distance(mid, bottom);
    frustum[3] = 1.0 * distance(mid, top);
    frustum[4] = near;
    frustum[5] = far;
}

// calcule la transformation GtoC
// GtoC = Global to Camera = Mire to Camera
void GLDisplay::calcTransformation(const cv::Mat & R, const cv::Mat & T)
{
    // GTOC Matrix
    gtoc[0][0] = R.at<double>(0,0);
    gtoc[1][0] = R.at<double>(1,0);
    gtoc[2][0] = R.at<double>(2,0);
    gtoc[3][0] = 0.0f;

    gtoc[0][1] = R.at<double>(0,1);
    gtoc[1][1] = R.at<double>(1,1);
    gtoc[2][1] = R.at<double>(2,1);
    gtoc[3][1] = 0.0f;

    gtoc[0][2] = R.at<double>(0,2);
    gtoc[1][2] = R.at<double>(1,2);
    gtoc[2][2] = R.at<double>(2,2);
    gtoc[3][2] = 0.0f;

    gtoc[0][3] = T.at<double>(0);
    gtoc[1][3] = T.at<double>(1);
    gtoc[2][3] = T.at<double>(2);
    gtoc[3][3] = 1.0f;
}

// draw X,Y,Z axis
void GLDisplay::drawAxis()
{
    static std::vector<float> VERTICES;
    static std::vector<float> COLORS;

    float taille = 10* squareSize;

    if( VERTICES.empty() )
    {
        // done the 1st time only

        // prepare vertices coordinates and color

        //
        //        Y ^
        //          |
        //          |
        //        O +---->
        //         /     X
        //      Z /
        //

        std::vector<float> O = {   0.0f,   0.0f,   0.0f };
        std::vector<float> X = { taille,   0.0f,   0.0f };
        std::vector<float> Y = {   0.0f, taille,   0.0f };
        std::vector<float> Z = {   0.0f,   0.0f, taille };

        std::vector<float> red   = { 1.0f, 0.0f, 0.0f };
        std::vector<float> green = { 0.0f, 1.0f, 0.0f };
        std::vector<float> blue  = { 0.0f, 0.0f, 1.0f };

        // red X axis
        COLORS.insert(COLORS.end(), red.begin(), red.end());
        COLORS.insert(COLORS.end(), red.begin(), red.end());
        VERTICES.insert(VERTICES.end(), O.begin(), O.end());
        VERTICES.insert(VERTICES.end(), X.begin(), X.end());
        // green Y axis
        COLORS.insert(COLORS.end(), green.begin(), green.end());
        COLORS.insert(COLORS.end(), green.begin(), green.end());
        VERTICES.insert(VERTICES.end(), O.begin(), O.end());
        VERTICES.insert(VERTICES.end(), Y.begin(), Y.end());
        // blue Z axis
        COLORS.insert(COLORS.end(), blue.begin(), blue.end());
        COLORS.insert(COLORS.end(), blue.begin(), blue.end());
        VERTICES.insert(VERTICES.end(), O.begin(), O.end());
        VERTICES.insert(VERTICES.end(), Z.begin(), Z.end());

        // fill vertex and color buffers
        // link vertex buffer and color buffer to vao

        glBindVertexArray(context.axis_vao);
        // vertices
        glBindBuffer(GL_ARRAY_BUFFER, context.axis_vertex_buffer);
        glBufferData(GL_ARRAY_BUFFER, VERTICES.size()*sizeof(VERTICES[0]), VERTICES.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(ATTRIB_POINT /*shader layout*/, 3 /*size*/, GL_FLOAT /*type*/, GL_FALSE /*normalized?*/, 0 /*stride*/, 0 /*array buffer offset*/);
        glEnableVertexAttribArray(ATTRIB_POINT);
        // color
        glBindBuffer(GL_ARRAY_BUFFER, context.axis_color_buffer);
        glBufferData(GL_ARRAY_BUFFER, COLORS.size()*sizeof(COLORS[0]), COLORS.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(ATTRIB_COLOR /*shader layout*/, 3 /*size*/, GL_FLOAT /*type*/, GL_FALSE /*normalized?*/, 0 /*stride*/, 0 /*array buffer offset*/);
        glEnableVertexAttribArray(ATTRIB_COLOR);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    // draw
    glBindVertexArray(context.axis_vao);
    glDrawArrays(GL_LINES, 0, VERTICES.size());
    glBindVertexArray(0);
}

/**
    Draw chessboard, origin at inner corner

    @param  w  chessboard width in squares
    @param  h  chessboard height in squares
    @param  sz  sqare size
*/
void GLDisplay::drawChessboard()
{
    // vertices position and color
    static std::vector<float> VERTICES;
    static std::vector<float> COLORS;

    if( VERTICES.empty() )
    {
        // done the 1st time only

        // prepare vertices coordinates and color
        for(int i = 0; i < patternWidth; i++){
            for(int j = 0; j < patternHeight; j++){
                // Coordinates values
                float i1 = (i-1)*squareSize;
                float i2 = i*squareSize;
                float j1 = (j-1)*squareSize;
                float j2 = j*squareSize;

                std::vector<float> p1({i1,j1,0}); // Top left
                std::vector<float> p2({i2,j1,0}); // Top right
                std::vector<float> p3({i1,j2,0}); // Bottom left
                std::vector<float> p4({i2,j2,0}); // Bottom right

                /*    p1 ___ p2
                 *      |\  |
                 *      | \ |
                 *      |__\|
                 *    p3     p4
                 */

                // Top Triangle
                VERTICES.insert(VERTICES.end(),p1.begin(),p1.end());
                VERTICES.insert(VERTICES.end(),p4.begin(),p4.end());
                VERTICES.insert(VERTICES.end(),p2.begin(),p2.end());
                // Bottom Triangle
                VERTICES.insert(VERTICES.end(),p1.begin(),p1.end());
                VERTICES.insert(VERTICES.end(),p3.begin(),p3.end());
                VERTICES.insert(VERTICES.end(),p4.begin(),p4.end());

                if(i%2 != j%2)  // White
                {
                    std::vector<float> c(18,1.f);
                    COLORS.insert(COLORS.end(),c.begin(),c.end());
                }
                else            // Black
                {
                    std::vector<float> c(18,0.f);
                    COLORS.insert(COLORS.end(),c.begin(),c.end());
                }
            }
        }

        // fill vertex and color buffers
        // link vertex buffer and color buffer to vao
        glBindVertexArray(context.chessboard_vao);
        // vertices
        glBindBuffer(GL_ARRAY_BUFFER, context.chessboard_vertex_buffer);
        glBufferData(GL_ARRAY_BUFFER, VERTICES.size()*sizeof(VERTICES[0]), VERTICES.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(ATTRIB_POINT /*shader layout*/, 3 /*size*/, GL_FLOAT /*type*/, GL_FALSE /*normalized?*/, 0 /*stride*/, 0 /*array buffer offset*/);
        glEnableVertexAttribArray(ATTRIB_POINT);
        // color
        glBindBuffer(GL_ARRAY_BUFFER, context.chessboard_color_buffer);
        glBufferData(GL_ARRAY_BUFFER, COLORS.size()*sizeof(COLORS[0]), COLORS.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(ATTRIB_COLOR /*shader layout*/, 3 /*size*/, GL_FLOAT /*type*/, GL_FALSE /*normalized?*/, 0 /*stride*/, 0 /*array buffer offset*/);
        glEnableVertexAttribArray(ATTRIB_COLOR);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    // draw
    glBindVertexArray(context.chessboard_vao);
    glDrawArrays(GL_TRIANGLES, 0, VERTICES.size());
    glBindVertexArray(0);
}

int GLDisplay::initGLDisplay()
{
    // options
    const char *title = "OpenGL display";
    frustum = new float[6];
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

    context.window = glfwCreateWindow(Window_Width, Window_Height, title, NULL, NULL);
    glfwMakeContextCurrent(context.window);
    glfwSwapInterval(1);

    // initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }

    // compile and link OpenGL program

    GLuint vert = compile_shader(GL_VERTEX_SHADER, vert_shader);
    GLuint frag = compile_shader(GL_FRAGMENT_SHADER, frag_shader);
    context.program = link_program(vert, frag);
    glDeleteShader(frag);
    glDeleteShader(vert);

    GLuint vert_tex = compile_shader(GL_VERTEX_SHADER, vert_shader_tex);
    GLuint frag_tex = compile_shader(GL_FRAGMENT_SHADER, frag_shader_tex);
    context.program_tex = link_program(vert_tex, frag_tex);
    glDeleteShader(frag_tex);
    glDeleteShader(vert_tex);

    // create buffers

    glGenBuffers(1, &context.axis_vertex_buffer);
    glGenBuffers(1, &context.chessboard_vertex_buffer);
    glGenBuffers(1, &context.texture_vertex_buffer);
    glGenBuffers(1, &context.axis_color_buffer);
    glGenBuffers(1, &context.chessboard_color_buffer);
    glGenBuffers(1, &context.texture_coord_buffer);


    glGenBuffers(1, &context.off_vertex_buffer);
    glGenBuffers(1, &context.off_color_buffer);

    glGenTextures(1, &context.texture);
    glBindTexture(GL_TEXTURE_2D, context.texture);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    // prepare vertrex array object (VAO)

    glGenVertexArrays(1, &context.axis_vao);
    glGenVertexArrays(1, &context.chessboard_vao);
    glGenVertexArrays(1, &context.texture_vao);
    glGenVertexArrays(1, &context.off_vao);


    // some GL settings

    glEnable(GL_DEPTH_TEST);
    glLineWidth(5.0f);

    // start main loop
    glfwSetKeyCallback(context.window, key_callback);

    return 0;
}

int GLDisplay::cleanGLDisplay()
{
    fprintf(stderr, "Exiting ...\n");

    // cleanup and exit

    glDeleteVertexArrays(1, &context.axis_vao);
    glDeleteVertexArrays(1, &context.chessboard_vao);
    glDeleteVertexArrays(1, &context.texture_vao);
    glDeleteVertexArrays(1, &context.off_vao);
    glDeleteBuffers(1, &context.axis_vertex_buffer);
    glDeleteBuffers(1, &context.chessboard_vertex_buffer);
    glDeleteBuffers(1, &context.texture_vertex_buffer);
    glDeleteBuffers(1, &context.axis_color_buffer);
    glDeleteBuffers(1, &context.chessboard_color_buffer);
    glDeleteBuffers(1, &context.texture_coord_buffer);
    glDeleteBuffers(1, &context.off_vertex_buffer);
    glDeleteBuffers(1, &context.off_color_buffer);
    glDeleteTextures(1, &context.texture);
    glDeleteProgram(context.program);
    glDeleteProgram(context.program_tex);

    glfwTerminate();
    delete[] frustum;

    return 0;
}