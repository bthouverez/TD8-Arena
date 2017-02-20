#ifndef CAMERA_ARENA_H
#define CAMERA_ARENA_H

#include "common.hpp"
#include "vec.h"
#include "mat.h"
#include <iostream>
#include <fstream>

class CameraArena{
public:
    CameraArena();
    CameraArena(const float width, const float height, const float rate, int device = 0);
    ~CameraArena();

    void show();
    void get();
    bool intrinsics(const int, const int, const float, const int);
    bool extrinsics(const int, const int, const float);

    void read(std::string filename);
    void write(std::string filename);

    cv::Mat mat(){return frame.clone();}
    cv::Mat amatrix(){return A.clone();}
    cv::Mat kmatrix(){return K.clone();}
    cv::Mat rmatrix(){return R.clone();}
    cv::Mat tmatrix(){return T.clone();}

    // OpenGL texture from frame
    GLuint background();
    void release();

    void frustum(int w, int h, float near = 1.f, float far = 10000.f);
    glm::vec3 unproject(glm::vec2 point, float sz);

    
    glm::mat4 projection(){return projection_matrix;}
    glm::mat4 view(){return view_matrix;}
    glm::mat4 gtoc(){return gtoc_matrix;}
    Point position(){return Point(T.at<double>(0),T.at<double>(1),T.at<double>(2));}

private:
    cv::VideoCapture camera;
    cv::Mat frame, A , K ,  R , T;
    GLuint texture;

    glm::mat3 intrinsic_matrix;
    glm::mat4 projection_matrix, gtoc_matrix, view_matrix;
};

#endif