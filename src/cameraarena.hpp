#ifndef CAMERA_ARENA_H
#define CAMERA_ARENA_H

#include "opencv2/opencv.hpp"
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

private:
    cv::VideoCapture camera;
    cv::Mat frame, A , K ,  R , T;
    //int nb;
    //float w, h, s;
};

#endif