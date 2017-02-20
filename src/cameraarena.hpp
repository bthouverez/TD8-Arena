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

    void frustum(int w, int h, float near = 200.f, float far = 10000.f);
    Point unproject(Point point, float sz);

    
    Transform projection(){return proj;}
    Transform view(){return Lookat(Point(0.f,0.f,0.f),Point(0.f,0.f,1.f),Vector(0.f,-1.f,0.f));}
    Transform gtoc(){return globaltocamera;}
    Point position(){return Point(T.at<double>(0),T.at<double>(1),T.at<double>(2));}

private:
    cv::VideoCapture camera;
    cv::Mat frame, A , K ,  R , T;
    GLuint texture;
    Transform cmatrix, proj, globaltocamera;
    //int nb;
    //float w, h, s;
};

#endif