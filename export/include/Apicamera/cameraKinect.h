
#ifndef 	__APICAM_CAMERAKINECT_H__
#define 	__APICAM_CAMERAKINECT_H__


#include "camera.h"
#include "kinect_utils.h"

#ifdef D_BUILDWINDLL
	#define DLL_EXPORT __declspec(dllexport)
#else
	#define DLL_EXPORT
#endif

namespace apicamera
{

/** 
*  \brief Manage a Kinect camera with libfreenect library.
*/
class DLL_EXPORT CameraKinect : public Camera
{
public :
	/**
	*  Class constructor. Make basic initialization.
	*/
	CameraKinect(void);

	/**
	*  Class destructor.
	*/
	~CameraKinect();

	/** @brief  image type */
	enum IMGTYPE
	{
		IMGTYPE_BEGIN = 0,
		IMGTYPE_RGB = 0, 	   // RGB image 
		IMGTYPE_DEPTH = 1,	   // Depth image
		IMGTYPE_IR_8BITS = 2,  // 8 bits IR image
		IMGTYPE_IR_10BITS = 3, // 10 bits IR image
		IMGTYPE_DISTANCE = 4,  // Depth image converted to distance
		IMGTYPE_END = 4
	};
	
	/** \brief  various camera parameters */
	enum CAMERA_PARAMETERS
	{
		PARAM_FIRST = 0, // must be the first parameter
		
		PARAM_IRMODE,  		 // kinect gives IR image, no RGB image
		PARAM_RGBMODE, 		 // kinect gives RGB image, no IR image
		PARAM_OUTPUTIMGTYPE, // kind of image returned by get1Frame
		PARAM_IRSCALEUP, 	 // IR image is scaled up
		PARAM_RGB_MANUAL_PARAMETERS, // RGB camera manual parameters
		PARAM_RGB_EXPOSURE, 	 // RGB camera exposure
		PARAM_RGB_SENSITIVITY, 	 // RGB camera sensitivity
	
		PARAM_LAST	// must be the last parameter
	};	

	/**
	*  Open a Kinect camera.
	*
	*  @param  numCam  Camera Id, from 0 to n-1.
	*  @param  param  opening parameters : mode and modulationFrq must be set.
	*
	*  @return value : 0 = ok, -1 = error.
	*/
	virtual int open( int, OpenParameters *param);
	virtual int open( int, unsigned int width, unsigned int height, unsigned int fRate,	unsigned int format) { return -1; /* not allowed for this kind of camera */ };
	
	/**
	*  Close camera device. Use MesaSR lib.
	*/
	virtual void close(void);

	/**
	*  Get one frame from the camera.
	*  Returns one image of output image type (see setParameter(PARAM_OUTPUTIMGTYPE,...)).
	*
	*  @return  Pointer to next image from camera. 
	*/
	virtual IplImage* get1Frame(void);

	/**
	*  Get next frame from camera and return a pointer to camera raw data.
	*
	*  @return  Pointer to raw data, depending of output image type parameter, see setParameter(PARAM_OUTPUTIMGTYPE,...). Original raw data pointer is of "void *" type.
	*/
	virtual void* getRawDataPtr(void);
	
	/**
	*  Display a list of all parameters.
	*/
	virtual void listParameters(void);

	/**
	*  Get parameter value.
	*
	*  @param  control  Parameter id, in CAMERA_PARAMETERS enum.
	*  @return  parameter value, -1 in case of error.
	*
	*  @bug  hope -1 is not a correct value for a parameter !
	*/
	virtual int getParameter(int control);

	/**
	*  Set parameter value.
	*
	*  @param  control  Parameter id, in CAMERA_PARAMETERS enum.
	*  @return  parameter value, -1 in case of error.
	*/
	virtual int setParameter(int control, int value);

	/**
	*  Load intrinsic parameters from file.
	*
	*  @param  fileName  Name of file to read from.
	*/
	virtual void loadIntrinsicParameters(const char *fileName);

	/**
	*  Load extrinsic parameters from file.
	*
	*  @param  fileName  Name of file to write to.
	*/
	virtual void loadExtrinsicParameters(const char *fileName);


	//--- cameraKinect specifics

	/**
	*  RGB image accessor.
	*  Returned image is 3 channels, 8 bits per channel.
	*  @return  RGB image. Do NOT release image !
	*/
	IplImage* getRGB(void)  { return lastFrames[IMGTYPE_RGB]; }
	
	/**
	*  Raw depth image accessor.
	*  Returned image is 1 channel, 16 bits per channel.
	*  Only 11 bits are used. Raw depth values in [0,2048].
	*  @return  Raw depth image. Do NOT release image !
	*/
	IplImage* getDepth(void)  { return lastFrames[IMGTYPE_DEPTH]; }
	
	/**
	*  IR image accessor.
	*  Returned image is either 1 channel 8 bits, or 1 channel 16 bits (10 bits
	*  used) image, according to current output image type.
	*  Turn IR on with setParameter() to get valid IR image !
	*  When IR is turned on, RGB image is not valid.
	*  @return  IR image. Do NOT release image !
	*/
	IplImage* getIR(void)  
	{ 
		if( IRMode == 1 ) 
			return lastFrames[IMGTYPE_IR_8BITS];
		else
			return lastFrames[IMGTYPE_IR_10BITS];}

	/**
	*  Distance image accessor (depth converted to meters).
	*  Returned image is 1 channel float.
	*  @return  Distance image. Do NOT release image !
	*/
	IplImage* getDistance(void)  { return lastFrames[IMGTYPE_DISTANCE]; }
	
	/**
	*  Convert depth image to color.
	*  @param  depth  1 channel depth image as returned by getDepth().
	*  @return  3 channels, 8 bits, color image. Must be released after use.
	*/
	IplImage* colorCodedDepth(IplImage* depth);

	/**
	*  Get camera parameter name.
	*
	*  @param  parId  Parameter Id, see CAMERA_PARAMETERS enum.
	*  @param  buffer  Buffer to store character string.
	*  @param  bufSize  Size of buffer.
	*/
	void getCameraParameterName( int parId, char* buffer, int bufSize);
	
	/**
	*  Record last RGB/IR and raw depth frames to file.
	*  File name pattern is Kinect-id-rgb-%06d.png for RGB image, and
	*  Kinect-id-depth-%06d.pgm for raw depth image.
	*/
	virtual void recordFrame(void);

	/**
	*  Get IR Mode status.
	*  @return  true if IR is turned on, false if RGB is turned on.
	*/
	bool isIRModeOn(void)  { if(IRMode == 0) return false; else return true; }

	/**
	*  Get depth camera calibration parameters.
	*  @return  pointer to calibration data.
	*/
	float* getIntrinsicKDepth(void)  { return intrinsicK_depth; }
	float* getIntrinsicADepth(void)  { return intrinsicA_depth; }
	float* getExtrinsicRDepth(void)  { return extrinsicR_depth; }
	float* getExtrinsicTDepth(void)  { return extrinsicT_depth; }

protected :
	/**
	*  Initialize undistortion maps, using camera intrinsic parameters.
	*/
	virtual void initUndistortMaps(void);


	//! pointers to differents images
	IplImage*  lastFrames[5];

	//! kind of image (depth, rgb, ir or distance) returned by get1Frame()
	int outputImageType;
	
	//! IR/RGB mode (0 = RGB mode, 1 = IR 8 bits mode, 2 = IR 10 bits mode)
	int IRMode;

	//! 10 bits IR scale up mode flag (0 = off, 1 = on)
	int IRScaleUpOn;

	// depth camera calibration parameters
	VEC4  intrinsicK_depth;	//! intrinsic distortion - 1x4 float vector
	MAT33  intrinsicA_depth; //!  intrinsic projection - 3x3 float matrix
	MAT33  extrinsicR_depth;	//!  extrinsic rotation - 3x3 float matrix
	VEC3  extrinsicT_depth;	//!  extrinsic translation - 1x3 float vector

	// depth image undistortion
	IplImage* undistortMapx_depth;
	IplImage* undistortMapy_depth;
};

} // namespace apicamera

#endif 	    // __APICAM_CAMERAKINECT_H__

