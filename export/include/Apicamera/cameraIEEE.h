#ifndef 	__APICAM_CAMERAIEEE_H__
#define 	__APICAM_CAMERAIEEE_H__
 

#include "camera.h"
#include <dc1394/dc1394.h>


/** \brief  extension to DC1394_FEATURE defined in /usr/include/dc1394/control.h */ 
enum DC1394_FEATURE_EXTENSION
{ 
		DC1394_MYFEATURE_ISO_SPEED = DC1394_FEATURE_MAX+1,
		DC1394_MYFEATURE_WHITE_BALANCE_UB_VALUE,
		DC1394_MYFEATURE_WHITE_BALANCE_VR_VALUE,
		DC1394_MYFEATURE_WHITE_SHADING_R_VALUE,
		DC1394_MYFEATURE_WHITE_SHADING_G_VALUE,
		DC1394_MYFEATURE_WHITE_SHADING_B_VALUE,				
};


namespace apicamera
{

/** 
*  \brief Manage a IEEE1394 camera using dc1394 library version 2.0.2-1. Linux only.
*/
class CameraIEEE : public apicamera::Camera
{
public :
	/**
	*  Class constructor. Make basic initialization.
	*/
	CameraIEEE();

	/**
	*  Class destructor.
	*/
	~CameraIEEE();

	/* prevoir un constructeur par copie */

	/**
	*  Open a IEEE camera device using dc1394 lib.
	*
	*  \param  numCam  Camera Id, from 0 to n-1.
	*  \param  param  opening parameters : width, height, fRate and format must be set. Look at open( int numCam, unsigned int width, unsigned int height, unsigned int fRate, unsigned int format) for more precision.
	*
	*  \return 0 if ok, -1 if it failed.
	*/
	virtual int open( int, OpenParameters *param);

	/**
	*  Open a IEEE camera device, using dc1394 lib.
	*
	*  \param  numCam  Camera Id, from 0 to n-1.
	*  \param  width  Camera resolution on X axis.
	*  \param  height  Camera resolution on Y axis
	*  \param  fRate  Framerate, use DC1394_FRAMERATE_... values defined in /usr/include/dc1394/video.h.
	*  \param  format  Image color coding, use DC1394_COLOR_CODING_... defined in /usr/include/dc1394/types.h. Only DC1394_COLOR_CODING_MONO8 is supported yet. 
	*
	*  \return 0 if ok, -1 if it failed.
	*/
	virtual int open( int, unsigned int width, unsigned int height, unsigned int fRate,	unsigned int format);

	/**
	*  Close camera device.
	*/
    virtual void close(void);

	/**
	*  Get one frame from the camera.
	*
	*  \return  Pointer to next image (converted to BGR) from camera.
	*/
	virtual IplImage* get1Frame(void);

	/**
	*  Get next frame from camera and return a pointer to camera raw data.
	*
	*  \return  Pointer to raw data. Original raw data pointer is of "unsigned char *" type.
	*/
	virtual void* getRawDataPtr(void);

	/**
	*  Display a list of all parameters.
	*/
	virtual void listParameters(void);

	/**
	*  Get parameter value.
	*
	*  \param  control  Parameter id. It's a DC1394_FEATURE_... value defined in /usr/include/dc1394/control.h or a DC1394_MYFEATURE_... value defined in cameraIEEE.h.
	*  \return  parameter value, -1 in case of error.
	*
	*  \bug  hope -1 is not a correct value for a parameter !
	*/
	virtual int getParameter(int control);

	/**
	*  Set parameter value.
	*
	*  \param  control  Parameter id. It's a DC1394_FEATURE_... value defined in /usr/include/dc1394/control.h or a DC1394_MYFEATURE_... value defined in cameraIEEE.h.
	*  \param  value  Value to assign to parameter.
	*  \return  0 if successfull, -1 in case of error.
	*/
	virtual int setParameter(int control, int value);

	/**
	*  Activate / de-activate image down-sampling.
	*  When activated, ouput image resolution is half real camera resolution on 2 axis.
	*
	*  \param  state  true = downsample on, false = downsample off.
	*  \return  0 = successfull, -1 = failed.
	*/
	virtual int setDownsampleMode(bool state);
	
protected :
	/**
	*  Set camera resolution and format.
	*
	*  \param  width  Resolution on X axis.
	*  \param  height  Resolution on Y axis.
	*  \param  format  Color coding format. Only MON08 supported now.
	*  \return  0 if successfull, -1 in case of error.
	*/
	int setResolutionAndFormat( unsigned int width, unsigned int height, unsigned int format);

	/**
	*  Set framerate.
	*
	*  \param  fRate  Framerate.
	*  \return  0 if successfull, -1 in case of error.
	*/
	int setFramerate(unsigned int fRate);

	/**
	*  Transform a bayerGR image into downsampled BGR image.
	*
	*  \param  buffer  bayerGR image.
	*  \param  bgr  downsampled BGR image, resolution is half bayerGR resolution on 2 axis.
	*/
	void bayer_downsampleBGR( const IplImage* buffer, IplImage* bgr);

	dc1394camera_t  *dc1394Camera;
	dc1394_t  *dc1394Context;
	unsigned char* rawDataBuffer;
};

} // namespace apicamera

#endif 	    // __APICAM_CAMERAIEEE_H__
