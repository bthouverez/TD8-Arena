#ifndef 	__APICAM_CHESSBOARD_CALIBRATION_H__
#define 	__APICAM_CHESSBOARD_CALIBRATION_H__
 

/** \brief Calibrate camera with a chessboard pattern.
 *
 */

#include "calibration.h"


class DLL_EXPORT ChessboardCalibration : public Calibration
{
public :

	/**
	*  Class constructor.
	*
	*  \param camPtr Pointer to the camera to calibrate.
	*  \param imgCountMax max image count to use for calibration.
	*  \param cbWidth chessboard width in squares.
	*  \param cbHeight chessboard height in squares.
	*  \param squareSize size of a chessboard square.
	*/
	ChessboardCalibration( apicamera::Camera *camPtr, unsigned int _imgCountMax, unsigned int _cbWidth, unsigned int _cbHeight, float _squareSize);

	/**
	*  Class destructor.
	*/
	virtual ~ChessboardCalibration();

	/* prevoir un constructeur par copie */
	
	/**
	*  Compute intrinsic parameters without user interaction, using openCV function.
	*
	*  \param windowMame OpenCV window name - If NULL, no display is done.
	*
	*  Return intrinsic parameters computing error. 
	*/
	virtual float findIntrinsicParameters(char *windowName);
	
	/**
	*  Get a frame from camera, then compute extrinsic parameters 
	*  using openCV function. Be sure that intrinsic parameters
	*  are loaded before calling this function. If a window name is
	*  provided, the camera image with detected corners is displayed.
	*
	*  \param dx x translation to apply to chessboard if not at origin.
	*  \param dy y translation to apply to chessboard if not at origin.
	*  \param windowMame OpenCV window name - If NULL, no display is done.
	*
	*  Return computing error.
	*/
	virtual float findExtrinsicParameters( float dx, float dy, char *windowName);

	/**
	*  Compute extrinsic parameters using an image. 
	*  Be sure that intrinsic parameters are loaded before calling this function.
	*
	*  \param dx  x translation to apply to chessboard if not at origin.
	*  \param dy  y translation to apply to chessboard if not at origin.
	*  \param image  image to use for calculation.
	*
	*  Return computing error.
	*/
	virtual float findExtrinsicParameters( float dx, float dy, IplImage *image);
	
	/**
	*  Calculate intrinsic parameters on current 2D corners set, 
	*  using openCV cvCalibrateCamera function.
	*  
	*  Return computing error. 
	*/
	float calcIntrinsicParameters(void);
	
	/**
	*  Increase internal 2D corners set index.
	*/
	void incCb2DCornersSetIndex(void)   
	{ 
		cb2DCornersSetIndex++; 
	};

	/**
	*  Decrease internal 2D corners set index.
	*/
	void decCb2DCornersSetIndex(void)
	{ 
		if( cb2DCornersSetIndex > 0 ) cb2DCornersSetIndex--;
	};

	/**
	*  Reset internal 2D corners set index.
	*/
	void resetCb2DCornersSetIndex(void)
	{ 
		cb2DCornersSetIndex = 0; 
	};
	
	/**
	*  Get internal 2D corners set index.
	*/
	unsigned int getCb2DCornersSetCount(void)
	{ 
		return cb2DCornersSetIndex; 
	};

	/**
	*  Try to add a 2D corners set to 2D points list, using next frame
	*
	*  Return  1 if corners set successfully added
	*          0 if adding failed because no corners were detected in image
	*         -1 if adding failed because points list is full 
	*/
	int add2DCornersSet(IplImage *image);	

protected :

	/**
	*  Find 2D points matching the chessboard 3D points in the given frame.
	*  
	*  Return true if successfull, false else. 
	*/
	int extract2DPoints( IplImage *image, CvPoint2D32f* pcb2DCorners);
	
	/**
	*  Calculate chessboard inner corners 3D positions.
	*  dx, dy : translation to apply to the chessboard if not at origin
	*  The result is in cb3DCorners[]. 
	*/
	void calc3DPoints( float dx, float dy);
	
	/**
	*  Calculate extrinsic parameters, using openCV cvFindExtrinsicCameraParams2 function.
	*  Use the current intrinsic parameters.
	*  
	*  Return computing error. 
	*/
	float calcExtrinsicParameters(void);
	
	/**
	*  Back project 3D points to 2D, using intrinsic and extrinsic camera parameters.
	*  
	*/
	void back2DProjection( int point_count, const CvPoint3D32f* object_points, CvPoint2D32f* projected_points, bool use_dist);
	
	/**
	*  Correct distortion.
	*  
	*/
	void distortion( float* pf, const float* disto);

	/**
	*  Back project 2D points to 3D, using intrinsic and extrinsic camera parameters.
	*  
	*/
	void back3DProjection( int point_count, CvPoint3D32f* object_points, const CvPoint2D32f* projected_points);
	void undistortion( const MAT33 a, const VEC4 k, const float u, const float v, float& rx, float& ry );
	void pixel_to_camera( VEC4 c_c, const VEC2 c_i);
	float error2D( int npts, CvPoint2D32f* pts2d1, CvPoint2D32f* pts2d2);
	float error3D( int npts, CvPoint3D32f* pts2d1, CvPoint3D32f* pts2d2);
	
	CvPoint2D32f* cb2DCorners;	// 2D coordinates of inner chessboard corners
	CvPoint3D32f* cb3DCorners;	// 3D coordinates of inner chessboard corners
	unsigned int cb2DCornersSetIndex;	// index of current 2D points set
	unsigned int imgCountMax;	// max frame count used for intrinsic calibration
	unsigned int cbWidth;		// chessboard width (in squares)
	unsigned int cbHeight;		// chessboard height (in squares)
	float squareSize;			// chessboard square size
	CvSize imageSize;			// image size in pixels
};
	
#endif 	    // __APICAM_CHESSBOARD_CALIBRATION_H__

