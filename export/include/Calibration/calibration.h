#ifndef 	__APICAM_CALIBRATION_H__
#define 	__APICAM_CALIBRATION_H__
 

/** \brief Abstract class for camera calibration.
 *
 */

#include "../Apicamera/camera.h"


class DLL_EXPORT Calibration
{
public :
	/**
	*  Class constructor.
	*  Basic initialization.
	*  \param camPtr Pointer to the camera to calibrate.
	*/
	Calibration(apicamera::Camera *camPtr);

	/**
	*  Class destructor.
	*/
	virtual  ~Calibration();

	/* prevoir un constructeur par copie */
	
	/**
	*  Save intrinsic parameters to file.
	*
	*/
	void saveIntrinsicParameters(const char *fileName);

	/**
	*  Save extrinsic parameters to file.
	*
	*/
	void saveExtrinsicParameters(const char *fileName);

	/**
	*  Load intrinsic parameters from file.
	*
	*/
	void loadIntrinsicParameters(const char *fileName);

	/**
	*  Load extrinsic parameters from file.
	*
	*/
	void loadExtrinsicParameters(const char *fileName);

	apicamera::Camera* getCamera(void)  { return camera; };
	
	virtual float findIntrinsicParameters(char *windowName) = 0;

	virtual float findExtrinsicParameters( float dx, float dy, char *windowName) = 0;
	
protected :
	apicamera::Camera* camera;
};
	
#endif 	    // __APICAM_CALIBRATION_H__

