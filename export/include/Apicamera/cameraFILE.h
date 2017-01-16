#ifndef 	__APICAM_CAMERAFILE_H__
#define 	__APICAM_CAMERAFILE_H__


#include "camera.h"

#include <stdio.h>
#include <string>

#ifdef D_API_LINUX		
#include <tiffio.h>	
#define DLL_EXPORT
#endif

#ifdef D_BUILDWINDLL
	#define DLL_EXPORT __declspec(dllexport)
#else
	#define DLL_EXPORT
#endif


namespace apicamera
{

/** 
*  \brief  Manage a pseudo-camera. Takes images from files.
*/
class DLL_EXPORT CameraFILE : public apicamera::Camera
{
public :
	/**
	*  Class constructor. Make basic initialization.
	*/
	CameraFILE();

	/**
	*  Class destructor.
	*/
	~CameraFILE();

	/* prevoir un constructeur par copie */

	virtual int open( int, unsigned int width, unsigned int height, unsigned int fRate,	unsigned int format) { return -1; /* not allowed */ };

	/**
	*  Open a FILE camera.
	*
	*  \param  numCam  Camera Id, from 0 to n-1.
	*  \param  param  opening parameters : fileNamePattern, indexMin, indexMax must be set.
	*
	*  \return value : 0 = ok, -1 = error.
	*/
	virtual int open( int, OpenParameters *param);

	/**
	*  Close camera device.
	*/
    virtual void close(void);

	virtual void* getRawDataPtr(void) { return NULL; }; // not yet implemented

	/**
	*  Get one frame from the camera.
	*
	*  \return  Pointer to next image from camera.
	*/
	virtual IplImage* get1Frame(void);
	
	enum CAMFILE_PARAMETERS 
	{ 
		PARAM_INDEX_MIN, PARAM_INDEX_MAX
	};

	/**
	*  Set file name pattern.
	*  Must be called before CameraFILE::get1Frame.
	*
	*  \param  pattern  File name pattern. Example : the pattern "image-%04u.png" opens files image-0000.png, image-0001.png, ...
	*/
	virtual void setFileNamePattern(const char *pattern);

	/**
	*  Display a list of all parameters.
	*/
	virtual void listParameters(void);

	/**
	*  Get parameter value.
	*
	*  \param  control  Parameter id in CAMFILE_PARAMETERS enum.
	*  \return  parameter value, -1 in case of error.
	*
	*  \bug  hope -1 is not a correct value for a parameter !
	*/
	virtual int getParameter(int control);

	/**
	*  Set parameter value.
	*
	*  \param  control  Parameter id in E_parameters enum.
	*  \param  value  Value to assign to parameter.
	*  \return  0 if successfull, -1 in case of error.
	*/
	virtual int setParameter(int control, int value);

#ifdef D_API_LINUX
	/**
	*  Open a 4-channels tiff image using libtiff.
	*  OpenCV "cvLoadImage" only loads 3 channels.
	*
	*  \param  filename  Name of file to read from.
	*  \return  Pointer to loaded image.
	*/
	virtual IplImage* load4ChannelsTiffToABGR(const char* filename);
#endif	

protected :
	std::string fileNamePattern;
	unsigned int nextImageIndex;
	unsigned int indexMin;
	unsigned int indexMax;
};

} // namespace apicamera

#endif 	    // __APICAM_CAMERAFILE_H__
