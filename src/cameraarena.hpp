#ifndef CAMERA_ARENA_H
#define CAMERA_ARENA_H

#include "opencv2/opencv.hpp"
#include <iostream>
// includes from liris-vision
#include "Apicamera/cameraUVC.h"
#include "Apicamera/cameraOPENCV.h"
#include "Calibration/chessboardcalibration.h"

// Constructed functions from Starling Export
static void cameraUVC_getFrame( apicamera::CameraUVC *camera, cv::Mat *out1)
{
    cv::Mat(camera->get1Frame()).copyTo(*out1);
}

static void showImage( const char* windowName, const std::string& scaling, float minValue, float maxValue, const cv::Mat *in)
{
    cv::Mat img;

    if( in == NULL || ( in->cols == 0 && in->rows == 0 ) )
    {
        // invalid image, display empty image
        const int w = 200;
        const int h = 100; 
        img = cv::Mat( h, w, CV_8UC3, cv::Scalar(0));
        cv::line( img, cv::Point( 0, 0), cv::Point( w-1, h-1), cv::Scalar(0,0,255), 2);
        cv::line( img, cv::Point( 0, h-1), cv::Point( w-1, 0), cv::Scalar(0,0,255), 2);
    }
    else if( scaling == "Auto" )
    {
        // scale image between its min and max values
        // convert input data to float
        in->convertTo( img, CV_32F);
        // normalized in [0,1] to display float image
        cv::normalize( img, img, 1.0, 0.0, cv::NORM_MINMAX);
    }
    else if( scaling == "Manual" )
    {
        in->copyTo(img);
        // scale image between the user defined min and max
        // truncate pixel values to [minValue, maxValue]
        cv::min( img, maxValue, img);
        cv::max( img, minValue, img);
        // scale values so that [minValue, maxValue] becomes [0,1]
        in->convertTo( img, CV_32F, 1/(maxValue-minValue), -minValue/(maxValue-minValue));
    }
    else
    {
        // display input image as it is
        img = *in;
    }

    cv::imshow( windowName, img);
}

class IntrinsicChessboardCalibrator
{
public:
    IntrinsicChessboardCalibrator( unsigned int _cbWidth, unsigned int _cbHeight, int _image_count, const char *_intrinsicFileName)
    {
        // initialize camera to store intrinsic parameters
        camera = new apicamera::CameraUVC();

        // initialize calibration
        calibrator = new ChessboardCalibration( camera, _image_count, _cbWidth, _cbHeight, 1.0f);
        intrinsicFileName = _intrinsicFileName;
        computingDone = false;
    }

    ~IntrinsicChessboardCalibrator()
    {
        delete calibrator;
        delete camera;
    }

    void processFrame( const cv::Mat *inImg, cv::Mat *intrinsicA, cv::Mat *intrinsicK, cv::Mat *error, cv::Mat *outImg)
    {
        // do the computing only once
        if( ! computingDone )
        {
            // accumulate data 
            if( (! inImg) || (! inImg->data) )
                return;
            inImg->copyTo(*outImg);
            IplImage currentImage(*outImg);
            if( calibrator->add2DCornersSet(&currentImage) == -1 )
            {
                // there is enough data, do intrinsic calibration
                printf("Computing intrinsic parameters ...\n");
                camera->intrinsicError = calibrator->calcIntrinsicParameters();

                // save intrinsic parameters to file
                camera->saveIntrinsicParameters(intrinsicFileName);
                printf( "Intrinsic parameters saved to file '%s'.\n", intrinsicFileName);

                computingDone = true;
            }
        }

        // copy intrinsic parameters to outputs
        cv::Mat A( 3, 3, CV_32FC1, camera->intrinsicA);
        A.copyTo(*intrinsicA);
        cv::Mat K( 1, 4, CV_32FC1, camera->intrinsicK);
        K.copyTo(*intrinsicK);
        cv::Mat E( 1, 1, CV_32FC1, &(camera->intrinsicError));
        E.copyTo(*error);
    }

protected:
    // camera is used only to store/load/save intrinsic/extrinsic parameters
    apicamera::CameraUVC *camera;

    ChessboardCalibration *calibrator;
    const char *intrinsicFileName;
    bool computingDone;
};

class ExtrinsicChessboardCalibrator
{
public:
    ExtrinsicChessboardCalibrator( unsigned int _cbWidth, unsigned int _cbHeight, float _squareSize, const char *_intrinsicFileName, const char *_extrinsicFileName)
    {
        // load intrinsic parameters
        camera = new apicamera::CameraUVC();
        camera->loadIntrinsicParameters(_intrinsicFileName);

        // initialize calibration
        calibrator = new ChessboardCalibration( camera, 1, _cbWidth, _cbHeight, _squareSize);
        extrinsicFileName = _extrinsicFileName;
    }

    ~ExtrinsicChessboardCalibrator()
    {
        delete calibrator;
        delete camera;
    }

    void processFrame( const cv::Mat *inImg, const cv::Mat *intrinsicA, const cv::Mat *intrinsicK, cv::Mat *translation, cv::Mat *rotation, cv::Mat *error, cv::Mat *outImg)
    {
        if( (! inImg) || (! inImg->data) )
            return;

        inImg->copyTo(*outImg);
        IplImage currentImage(*outImg);

        // set intrinsic parameters if provided through block inputs
        if( intrinsicA )
        {
            cv::Mat A( 3, 3, CV_32FC1, camera->intrinsicA);
            intrinsicA->copyTo(A);
        }
        if( intrinsicK )
        {
            cv::Mat K( 1, 4, CV_32FC1, camera->intrinsicK);
            intrinsicK->copyTo(K);
        }

        // compute extrinsic parameters
        camera->extrinsicError = calibrator->findExtrinsicParameters( 0.0f, 0.0f, &currentImage);

        // save extrinsic parameters to file
        camera->saveExtrinsicParameters(extrinsicFileName);

        // copy extrinsic parameters and error to outputs
        cv::Mat T( 1, 3, CV_32FC1, camera->extrinsicT);
        T.copyTo(*translation);
        cv::Mat R( 3, 3, CV_32FC1, camera->extrinsicR);
        R.copyTo(*rotation);
        cv::Mat E( 1, 1, CV_32FC1, &(camera->extrinsicError));
        E.copyTo(*error);
    }

    // camera is used only to store/load/save intrinsic/extrinsic parameters
    apicamera::CameraUVC * camera;

protected:
    ChessboardCalibration *calibrator;
    const char *extrinsicFileName;
};

class CameraArena{
public:
    CameraArena();
    CameraArena(const float width, const float height, const float rate, int device = 0);
    ~CameraArena();

    void show();
    void get();
    bool intrinsics(const int, const int, const float, const int);

private:
    //ExtrinsicChessboardCalibrator * extrinsics;
    //IntrinsicChessboardCalibrator * intrinsics;
    
    apicamera::OpenParameters parameters;
    apicamera::CameraUVC camerauvc;

    cv::VideoCapture camera;

    cv::Mat frame, A , K ,  R , T;
};

#endif