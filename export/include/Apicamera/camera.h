
#ifndef 	__APICAM_CAMERA_H__
#define 	__APICAM_CAMERA_H__

#include <cv.h>
#include <locale.h>
#include <string>

#include "Common/mat44.h"


#ifdef D_API_WIN32
	#define __func__  __FUNCTION__
#endif

#ifdef D_BUILDWINDLL
	#define DLL_EXPORT __declspec(dllexport)
#else
	#define DLL_EXPORT
#endif


namespace apicamera
{

/** 
*  @brief Parameters for Camera::open method.
*/
class DLL_EXPORT OpenParameters
{
public:
	/**
	*  Set default values.
	*/
	OpenParameters(void)
	{
		width = 320;
		height = 240;
		fRate = 30;
		UVCformat = 1448695129;  // = V4L2_PIX_FMT_YUYV	
		IEEEformat = 352;  // = DC1394_COLOR_CODING_MONO8
		FILEFileNamePattern = "./img-%04u.png";
		FILEIndexMin = 1;
		FILEIndexMax = 50;
		SWRmode = 2325;  // = AM_COR_FIX_PTRN|AM_CONV_GRAY|AM_DENOISE_ANF|AM_CONF_MAP|AM_MEDIAN	
		SWRmodulationFrq = 1;  // = MF_30MHz
		NETSrvIP = "127.0.0.1";
		NETPort = 3200;
		NETBgSubCtrl = 0x0002; // = BG_CTRL_RGB_IMAGE
	}

	virtual void allowDynamicCast(void) { };
	
	// general purpose parameters
	unsigned int width;		//!<  resolution on X axis.
	unsigned int height;	//!<  resolution on Y axis. 
	unsigned int fRate;		//!<  framerate in fps.
	
	// cameraUVC specific parameters
	unsigned int UVCformat;	//!<  V4L Pixel format V4L2_PIX_FMT_... defined in /usr/include/linux/videodev2.h
	
	// cameraIEEE specific parameters
	unsigned int IEEEformat;	//!<  DC1394_COLOR_CODING_... defined in /usr/include/dc1394/types.h
	
	// cameraFILE specific parameters
	std::string FILEFileNamePattern; //!<  File name pattern like "image-%04u.png" (image-0000.png, image-0001.png ...)
	unsigned int FILEIndexMin; 	//!<  Min index value
	unsigned int FILEIndexMax;	//!<  Max index value
	
	// cameraSwissRanger specific parameters
	int SWRmode;	//!<  acquire mode, see SR_SetMode in MesaSR lib doc
				// mode = AM_COR_FIX_PTRN|AM_CONV_GRAY|AM_DENOISE_ANF|AM_CONF_MAP|AM_MEDIAN is a good value
	int SWRmodulationFrq;	//!<  Modulation Frequency as defined in Mesa API. Rewrite in cameraSwissRanger.h to see all modes. Default MF_30MHz.
				// modulationFrq = MF_30MHz is a good value

	// cameraNET specific parameters
	std::string NETSrvIP;	// Image server IP address
	int NETPort;			// Port number used by image server
	int NETBgSubCtrl;		// Server configuration bit field
};


/** 
*  @brief Virtual Camera Class.
*/
class DLL_EXPORT Camera
{
public :
	/**
	*  Class constructor.
	*/
	Camera();

	/**
	*  Class destructor.
	*/
	virtual ~Camera();

	/* prevoir un constructeur par copie */

	/**
	*  Open a camera device.
	*
	*  @param  numCam  Camera Id, from 0 to n-1.
	*  @param  param  Opening parameters.
	*
	*  @return 0 if ok, -1 if failed.
	*/
	virtual int open( int numCam, OpenParameters *param) = 0;

	/**
	*  Open a camera device, taking parameters from a configuration file.
	*
	*  @param  numCam  Camera Id, from 0 to n-1.
	*  @param  fileName  Configuration file including opening parameters.
	*
	*  @return 0 if ok, -1 if failed.
	*/
	virtual int open( int numCam, const char *fileName);

	/**
	*  Open a camera device. For fast opening and compatibility only.
	*
	*  @param  numCam  Camera Id, from 0 to n-1.
	*  @param  width  Camera resolution on X axis.
	*  @param  height  Camera resolution on Y axis
	*  @param  fRate  Framerate in fps.
	*  @param  format  Image format, depend on camera type. 
	*
	*  @return 0 if ok, -1 if it failed.
	*/
	virtual int open( int, unsigned int width, unsigned int height, unsigned int fRate, unsigned int format);

	/**
	*  Close camera device.
	*/
	virtual void close(void);

	/**
	*  Get next frame from camera and return a pointer to camera raw data.
	*
	*  @return  Pointer to raw data. Depends on camera type.
	*/
	virtual void* getRawDataPtr(void) = 0;

	/**
	*  Get one frame from the camera.
	*
	*  @return  Pointer to next image from camera. Must not be released by user !
	*/
	virtual IplImage* get1Frame(void) = 0;

	/**
	*  Show camera frames in a window.
	*  @param  recordingOn  If true, images are recorded to files, using 
	*  recordFrame().
	*/
	virtual void show(bool recordingOn = false);

	/**
	*  Display camera parameters.
	*/
	virtual void listParameters(void) 
		{ printf("No parameter available for this camera.\n"); }

	/**
	*  Get one camera parameter value.
	*
	*  @param  paramId  Parameter id.
	*  @result  Parameter value.
	*/
	virtual int getParameter(int paramId)  { return -1; } // not implemented

	/**
	*  Set one camera parameter value.
	*
	*  @param  paramId  Parameter id.
	*  @param  value  Parameter value.
	*  @result  Parameter value.
	*/
	virtual int setParameter(int paramId, int value)  { return -1; } // not implemented

	/**
	*  Set one or more camera parameter value.
	*  Use a string of (paramId,value) pairs, like "1 265 3 25"
	*
	*  @param  params  String of (paramId,value) pairs.
	*/
	virtual void setParameter( const std::string& params);

	/**
	*  Get camera name.
	*
	*  @return  Pointer to camera name.
	*/
	virtual const char* getName(void)  { return name; }

	/**
	*  Get camera model.
	*
	*  @return  camera model.
	*/
	virtual const std::string& getModel(void)  { return model; }

	/**
	*  Set camera active state. Used by Capture to activate/deactivate camera.
	*
	*  @param  state  true = active, false = inactive
	*/
	virtual void setActive(bool state)  { active = state; }

	/**
	*  Get active state of camera.
	*
	*  @return  true = active, false = inactive.
	*/
	virtual bool isActive(void)  { return active; }

	/**
	*  Save intrinsic parameters to file.
	*
	*  @param  fileName  Name of file to write to.
	*/
	virtual void saveIntrinsicParameters(const char *fileName);

	/**
	*  Save extrinsic parameters to file.
	*
	*  @param  fileName  Name of file to write to.
	*/
	virtual void saveExtrinsicParameters(const char *fileName);

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

	/**
	*  Set/unset downsample mode. 
	*  When downsampling is on, images returned by get1Frame() are half current
	*  camera resolution on X Y.
	*
	*  @param  state  New downsample-mode state value.
	*  @return  0 if ok, -1 if failed.
	*/
	virtual int setDownsampleMode(bool state)  { return -1; /* failed */ }

	/**
	*  Get downsample mode. 
	*
	*  @return  downsample mode.
	*/
	virtual bool isDownsampleModeOn(void)  { return fDownsample; }

	/**
	*  Turn undistortion on or off.
	*  Intrinsic parameters must have been loaded by loadIntrinsicParameters() 
	*  before turning undistortion on !
	*
	*  @param  state  true = undistortion on, false = undistortion off.
	*  @return  0 = successfull, -1 = failed.
	*/
	virtual int setUndistortMode(bool state);

	/**
	*  Get undistort mode. 
	*
	*  @return  undistort mode.
	*/
	virtual bool isUndistortModeOn(void)  { return fUndistort; }

	/**
	*  Undistort image, using precalculated undistortion map based on camera
	*  intrinsic parameters. Undistortion is done in place.
	*
	*  @param  mapx  map of x-coordinates (see OpenCV cvRemap function)
	*  @param  mapy  map of y-coordinates (see OpenCV cvRemap function)
	*  @param  frame  image to undistort, must be writable.
	*  @return  frame pointer
	*/
	virtual IplImage* undistortFrame( IplImage* frame, const CvArr* mapx = NULL, const CvArr* mapy = NULL);

	/**
	*  Give real camera resolution.
	*
	*  @return  Real camera resolution on X and Y axis.
	*/
	virtual CvSize getResolution(void)  
		{ return cvSize( frameWidth, frameHeight); }
		
	/**
	*  Display content of OpenParameters struct.
	*
	*  @param  param  Parameters to display.
	*/
	void displayOpenParameters(const OpenParameters* param);
	
	/**
	*  Load OpenParameters struct from file.
	*
	*  @param  param  Parameters struct to fill.
	*  @param  fileName  Configuration file name.
	*/
	int loadOpenParametersFromFile( OpenParameters* param, const char *fileName);
	
	/**
	*  Record last frame(s) to file.
	*  File name pattern is cameraname-%06d.png or cameraname-%06d.pgm.
	*/
	virtual void recordFrame(void);
	
	/**
	*  Replacement for bugged config_lookup_int() from libconfig.
	*  Look at libconfig documentation for more information.
	*/
	//int config_lookup_int32( const config_t *config, const char *path, int *value);


	// calibration parameters
	VEC4  intrinsicK;	///<  intrinsic distortion - 1x4 float vector
	MAT33  intrinsicA;	///<  intrinsic projection - 3x3 float matrix
	MAT33  extrinsicR;	///<  extrinsic rotation - 3x3 float matrix
	VEC3  extrinsicT;	///<  extrinsic translation - 1x3 float vector
	float intrinsicError; ///<  error in intrinsic parameters computing
	float extrinsicError; ///<  error in extrinsic parameters computing

protected :
	/**
	*  Initialize undistortion maps, using camera intrinsic parameters.
	*/
	virtual void initUndistortMaps(void);

	char name[255];
	std::string model;
	unsigned int frameWidth;	///<  real camera resolution on x axis
	unsigned int frameHeight;	///<  real camera resolution on y axis
	unsigned int framerate;
	unsigned int frameFormat;
	IplImage* lastFrame;
	bool active;
	bool fDownsample;	///<  flag, output image resolution is half real camera resolution
	bool fUndistort;	///<  flag, undistort image using intrinsic distortion coeffs
	IplImage* undistortMapx;
	IplImage* undistortMapy;

	//! configuration parameters string
	std::string setParameterString;
};

} // namespace apicamera

#endif      //__APICAM_CAMERA_H__
