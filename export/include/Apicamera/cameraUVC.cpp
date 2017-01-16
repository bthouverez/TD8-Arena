#include "cameraUVC.h"

#include <stdio.h>
#include <string.h>
#include <highgui.h>

#include <sys/ioctl.h>


// the class factories

extern "C" DLL_EXPORT apicamera::Camera* create(void)
{
    return new apicamera::CameraUVC;
}

extern "C" DLL_EXPORT void destroy(apicamera::Camera* p) 
{
    delete p;
}


namespace apicamera
{

//-------------------------------------------------------------------

CameraUVC::CameraUVC()
{
	//printf( "ici CameraUVC::CameraUVC()\n");
	videoIn = NULL;
}

//-------------------------------------------------------------------

CameraUVC::~CameraUVC()
{
	//printf( "ici CameraUVC::~CameraUVC()\n");
	close();
}

//-------------------------------------------------------------------

int CameraUVC::open( int numCam, unsigned int width, unsigned int height, unsigned int fRate, unsigned int format)
{
	char videodevice[32];
	FILE* fd;

	sprintf( videodevice, "/dev/video%d", numCam);

	// teste si le fichier de péripérique est valide
	// avant appel a init_videoIn pour eviter sortie brutale
	fd = fopen( videodevice, "r+");
	if ( fd  == NULL)
	{
		fprintf( stderr, "WARNING: Can't open device %s.\n", videodevice);
		return -1;
	}
	fclose( fd );

	videoIn = new struct vdIn;
	if( init_videoIn( videoIn, (char*) videodevice, width, height, fRate, format, 1, NULL) < 0 )
	{
		fprintf( stderr, "WARNING: camera UVC %d not detected.\n", numCam);
   		delete videoIn;
		videoIn = NULL;
		return -1;
	}
	
	// look for camera model
	struct v4l2_input getinput;
	memset( &getinput, 0, sizeof(struct v4l2_input));
	getinput.index = 0;
	ioctl( getVdIn()->fd, VIDIOC_ENUMINPUT, &getinput);
	model.assign( (char*) getinput.name);

	// initializations
	frameWidth = width;
	frameHeight = height;
	framerate = fRate;
	frameFormat = format;

	// memory allocation for BGR image !!
	lastFrame = cvCreateImage( cvSize( frameWidth, frameHeight), IPL_DEPTH_8U, 3);
	if( lastFrame == NULL )
	{
		fprintf( stderr, "ERROR: camera %d : cvCreateImage failed.\n", numCam);
		return -1;
	}

	// si tout c'est bien passe ...
	sprintf( name, "UVC-%d", numCam);
	fprintf( stderr, "SUCCESS: %s detected.\n", name);
	setActive(true);
	initLut();

	return 0;
}

//-------------------------------------------------------------------

int CameraUVC::open( int numCam, OpenParameters *param)
{
	return open( numCam, param->width, param->height, param->fRate, param->UVCformat);		
}

//-------------------------------------------------------------------

void CameraUVC::close(void) 
{
	if( videoIn != NULL )
	{
		close_v4l2( videoIn );
		delete videoIn;
		videoIn = NULL;
	}

	Camera::close();
}

//-------------------------------------------------------------------

void* CameraUVC::getRawDataPtr(void)
{
	if( uvcGrab( videoIn ) < 0 )
	{
		 fprintf( stderr, "%s : getRawDataPtr failed.\n", name);
		 return NULL;
	}

	// videoIn->framebuffer is an unsigned char*
   	return (void*) videoIn->framebuffer;
}

//-------------------------------------------------------------------

IplImage* CameraUVC::get1Frame(void)
{
	unsigned char *rawData = NULL;
	
	rawData = (unsigned char *) getRawDataPtr();

	if( rawData == NULL )
	{
		fprintf( stderr, "ERROR in CameraUVC::get1Frame : failed to get new frame.\n");
		return lastFrame;
	}	
	
	if( ! isDownsampleModeOn() )
		Pyuv422tobgr24( rawData, (unsigned char *) lastFrame->imageData, frameWidth, frameHeight);
	else
		Pyuv422tobgr444DownSample( rawData, (unsigned char *) lastFrame->imageData, frameWidth, frameHeight);
		
	if( isUndistortModeOn() )
		undistortFrame(lastFrame);
	
	return  lastFrame;
}

//-------------------------------------------------------------------

int CameraUVC::getParameter(int control)
{
	return v4l2GetControl( videoIn, control);
}

//-------------------------------------------------------------------

int CameraUVC::setParameter(int control, int value)
{
	return v4l2SetControl( videoIn, control, value);
}

//-------------------------------------------------------------------

void CameraUVC::listParameters(void)
{
	// parameters
	enum_controls( videoIn->fd );

	// frame formats
	printf("Frame Formats :\n");
	enum_frame_formats( videoIn->fd, NULL, 0);
}

//-------------------------------------------------------------------

int CameraUVC::setDownsampleMode(bool state)
{
	if( state == isDownsampleModeOn() )
		return 0;
		// already in the right mode, nothing to do

	// resize image	storage, allocate memory for BGR image
	if( lastFrame != NULL )
		cvReleaseImage(&lastFrame);
	
	CvSize outputResolution;
	if( state )
		outputResolution = cvSize( frameWidth/2, frameHeight/2);
	else
		outputResolution = cvSize( frameWidth, frameHeight);
	
	lastFrame = cvCreateImage( outputResolution, IPL_DEPTH_8U, 3);
	if( lastFrame == NULL )
	{
		fprintf( stderr, "ERROR in CameraUVC::setDownsampleMode : cvCreateImage failed.\n");
		return -1;
	}
	
	fDownsample = state;
	
	// modify undistortion maps because resolution changed
	initUndistortMaps();

	return 0;
}

//-------------------------------------------------------------------

} // namespace apicamera
