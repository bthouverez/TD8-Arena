#include "cameraOPENCV.h"

#include <stdio.h>
#include <string.h>


// the class factories

extern "C" DLL_EXPORT apicamera::Camera* create(void)
{
    return new apicamera::CameraOPENCV;
}

extern "C" DLL_EXPORT void destroy(apicamera::Camera* p) 
{
    delete p;
}


namespace apicamera
{

//-------------------------------------------------------------------

CameraOPENCV::CameraOPENCV()
{
#ifdef _DEBUG
	printf( "entering CameraOPENCV::%s ...\n", __func__);
#endif

	idCamera = NULL;
}

//-------------------------------------------------------------------

CameraOPENCV::~CameraOPENCV()
{
#ifdef _DEBUG
	printf( "entering CameraOPENCV::%s ...\n", __func__);
#endif
	close();
}

//-------------------------------------------------------------------

int CameraOPENCV::open( int numCam, OpenParameters *param)
{
	idCamera = cvCreateCameraCapture(CV_CAP_ANY+numCam);

	if( !idCamera )
	{
		fprintf( stderr, "WARNING: camera %d not detected.\n", numCam);

		return -1;
	}

	sprintf( name, "OPENCV-%d", numCam);

	fprintf( stderr, "SUCCESS: %s detected \n", name);
	setActive(true);
    cvSetCaptureProperty( idCamera, CV_CAP_PROP_FRAME_WIDTH, (double) param->width);
    cvSetCaptureProperty( idCamera, CV_CAP_PROP_FRAME_HEIGHT, (double) param->height);

    //la ligne ci-dessous provoque un message d'erreur
    //"HIGHGUI ERROR: V4L: setting property #5 is not supported"
    // sous LINUX 64bits / opencv 1.0
    cvSetCaptureProperty( idCamera, CV_CAP_PROP_FPS, (double) param->fRate);

	// get a frame to set some parameters
	IplImage* frame = get1Frame();
	frameWidth = frame->width;
	frameHeight = frame->height;
	framerate = param->fRate;
    frameFormat = 0;	    

	return 0;
}

//-------------------------------------------------------------------

void CameraOPENCV::close(void)
{
	// Release the capture device
	if( idCamera != NULL )
		cvReleaseCapture(&idCamera);
		
	idCamera = NULL;
	
	apicamera::Camera::close();
}

//-------------------------------------------------------------------

void* CameraOPENCV::getRawDataPtr(void)
{
   	// IplImage* cvQueryFrame(CvCapture* capture)
   	return (void*) cvQueryFrame( idCamera );
}

//-------------------------------------------------------------------

IplImage* CameraOPENCV::get1Frame(void)
{
	IplImage *rawData;
	
	if( lastFrame != NULL )
		cvReleaseImage(&lastFrame);
		
	rawData = (IplImage*) getRawDataPtr();
	if( rawData != NULL )
		lastFrame = cvCloneImage(rawData);

	if( isUndistortModeOn() )
		undistortFrame(lastFrame);
	
	return  lastFrame;
}

//-------------------------------------------------------------------

} // namespace apicamera

