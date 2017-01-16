
#ifndef 	__APICAM_CAMERAOPENCV_H__
#define 	__APICAM_CAMERAOPENCV_H__

#include "camera.h"

#include <highgui.h>

#ifdef D_BUILDWINDLL
	#define DLL_EXPORT __declspec(dllexport)
#else
	#define DLL_EXPORT
#endif

namespace apicamera
{

/** 
*  \brief Manage a camera with OPENCV lib.
*/
class DLL_EXPORT CameraOPENCV : public apicamera::Camera
{
public :
	/**
	*  Class constructor. Make basic initialization.
	*/
	CameraOPENCV();

	/**
	*  Class destructor.
	*/
	virtual ~CameraOPENCV();

	/* prevoir un constructeur par copie */

	/**
	*  Open a camera device using OpenCV lib.
	*
	*  \param  numCam  Camera Id, from 0 to n-1.
	*  \param  param  opening parameters : width, height and fRate must be set.
	*                   format is not used for this kind of camera.
	*
	*  \return 0 if ok, -1 if it failed.
	*/
	virtual int open( int, OpenParameters *param);

	/**
	*  Close camera device.
	*  \bug open + close + open doesn't work. bug in cvReleaseCapture ?
	*/
	virtual void close(void);

	/**
	*  Get next frame from camera and return a pointer to camera raw data.
	*
	*  \return  Pointer to raw data. Original raw data pointer is of "IplImage*" type.
	*/
	virtual void* getRawDataPtr(void);

	/**
	*  Get one frame from the camera.
	*
	*  \return  Pointer to next image from camera. Must not be released by user !
	*/
	virtual IplImage* get1Frame(void);

	/**
	*  Get OpenCV Id of camera.
	*
	*  \return  OpenCV id.
	*/
	CvCapture* getId(void);

protected :

	//! OpenCV id of camera
	CvCapture *idCamera;
};

} // namespace apicamera

#endif      //__APICAM_CAMERAOPENCV_H__
