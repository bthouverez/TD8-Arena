#ifndef 	__APICAM_CAPTURE_H__
#define 	__APICAM_CAPTURE_H__

#include "camera.h"
#include "plugin.h"


#include <stdio.h>
#include <vector>
#include <highgui.h>


#ifdef D_BUILDWINDLL
	#define DLL_EXPORT __declspec(dllexport)
#else
	#define DLL_EXPORT
#endif


namespace apicamera
{

/** 
*  Manage multiple camera at the same time.
*/
class DLL_EXPORT Capture
{
public :
	/**
	*  Class constructor.
	*/
	Capture(Plugin<apicamera::Camera> *_camlib);

	/**
	*  Class destructor.
	*/
	~Capture();

	/**
	*  \brief Open a capture session.
	*  Detect all connected cameras, and set them as active.
	*  A configuration file can be use instead of OpenParameters structure.
	*
	*  \param  param  Pointer to parameters structure needed to open camera.
	*  \param  fileName  Configuration file used to configure camera.
	*  \return  number of detected cameras.
	*/
	virtual int open( apicamera::OpenParameters *param, 
		const char *fileName = NULL);

	/**
	*  \brief Close capture session.
	*  Close each camera.
	*/
	void close(void);

	/**
	*   Read a frame per active camera.
	*
	*   \return  List of pointer to IplImage, one IplImage per active camera.
	*/
	std::vector<IplImage*> getFrames(void);

	/**
	*   Read a frame from one camera.
	*
	*   \param  camNum  Camera id, from 0 to N-1.
	*   \return  Pointer to image read from camera.
	*/
	IplImage* getFrameFromCam(unsigned int);

	/**
	*  Get raw image data from a camera.
	*
	*  \param  camNum  Camera id, from 0 to N-1.
	*  \return  Pointer to raw image data.
	*/
	void* getRawDataPtrFromCam(unsigned int);

	/**
	*  Get a pointer to a camera object, to use special capabilities of some
	* cameras.
	*
	*  \param  camNum  Camera id, from 0 to N-1.
	*  \return  Pointer to camera.
	*/
	Camera* getCameraPtr(unsigned int camNum);

	/**
	*  Open viewing window for all active cameras frame.
	*/
	void show(void);

	/**
	*  Get detected cameras number (including inactive cameras).
	*
	*  \return  detected cameras number.
	*/
	int getCameraNumber(void);

	/**
	*  Get parameter value from one camera.
	*
	*  \param  camNum  Camera id, from 0 to N-1.
	*  \param  control  Parameter id. Depends on camera type.
	*  \return  parameter value, -1 in case of error.
	*
	*  \bug  hope -1 is not a correct value for a parameter !
	*/
	int getParameter( unsigned int camNum, int control);

	/**
	*  Set parameter value for all cameras.
	*
	*  \param  control  Parameter id. Depends on camera type.
	*  \param  value  Value to assign to parameter.
	*  \return  0 if successfull, -1 in case of error.
	*/
	int setParameter( int control, int value);

	/**
	*  Display list of parameters of one camera.
	*
	*  \param  camNum  Camera id, from 0 to N-1.
	*  \return  -1 = error, 0 = ok. 
	*/
	int listParameters( unsigned int camNum);

	/**
	*  Activate or de-activate a camera.
	*
	*  \param  camNum  Camera id, from 0 to N-1.
	*  \param  state  true = active, false = inactive.
	*/
	void setActiveCam( unsigned int, bool);

protected :

	std::vector<Camera*> camList;		///<  camera list
	Plugin<apicamera::Camera> *camlib;
};

} // namespace apicamera

#endif      //	__APICAM_CAPCameraURE_H__
