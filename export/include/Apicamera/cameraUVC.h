#ifndef 	__APICAM_CAMERAUVC_H__
#define 	__APICAM_CAMERAUVC_H__


#include "camera.h"
#include "UVCLib/uvc-release.h"


namespace apicamera
{

/** 
*  \brief Manage a UVC camera using a library build from luvcview sources (use V4L2 API). Linux only.
*/
class CameraUVC : public apicamera::Camera
{
public :
	/**
	*  Class constructor. Make basic initialization.
	*/
	CameraUVC();

	/**
	*  Class Destructor.
	*/
	~CameraUVC();

	/* prevoir un constructeur par copie */

	/**
	*  Open a UVC camera device using luvcview based library.
	*
	*  \param  numCam  Camera Id, from 0 to n-1.
	*  \param  param  opening parameters : width, height, fRate and format must be set. Look at open( int numCam, unsigned int width, unsigned int height, unsigned int fRate, unsigned int format) for more precision.
	*
	*  \return 0 if ok, -1 if it failed.
	*/
	virtual int open( int, OpenParameters *param);

	/**
	*  Open a UVC camera device using luvcview based library.
	*
	*  \param  numCam  Camera Id, from 0 to n-1.
	*  \param  width  Camera resolution on X axis.
	*  \param  height  Camera resolution on Y axis
	*  \param  fRate  Framerate in fps.
	*  \param  format  V4L Pixel format V4L2_PIX_FMT_... defined in /usr/include/linux/videodev2.h
	*
	*  \return 0 if ok, -1 if it failed.
	*/
	virtual int open( int, unsigned int width, unsigned int height, unsigned int fRate,	unsigned int format);

	/**
	*  Close a camera device.
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
	*  Print all parameters.
	*/
	virtual void listParameters(void);

	/**
	*  Get parameter value.
	*
	*  \param  control  Parameter id. It's a V4L2_CID_... id defined in /usr/include/linux/videodev2.h.
	*  \return  parameter value, -1 in case of error.
	*
	*  \bug  hope -1 is not a correct value for a parameter !
	*/
	virtual int getParameter(int control);

	/**
	*  Set parameter value.
	*
	*  \param  control  Parameter id. It's a V4L2_CID_... id defined in /usr/include/linux/videodev2.h.
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

	/**
	*  Get a pointer to UVCLib main data structure, defined in v4l2uvc.h.
	*
	*  \return  pointer to vdIn structure.
	*/
	virtual struct vdIn *getVdIn(void)  { return videoIn; };

protected :
	struct vdIn *videoIn;
};

} // namespace apicamera

#endif 		// __APICAM_CAMERAUVC_H__
