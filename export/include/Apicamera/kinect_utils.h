#include <cv.h>
#include "libfreenect/include/libfreenect.h"


const int KINECT_IMG_WIDTH = 640;	// depth and color image width
const int KINECT_IMG_HEIGHT = 480; // depth and color image height
const int KINECT_IR_WIDTH = 640;	// IR image width
const int KINECT_IR_HEIGHT = 488; // IR image height


/** 
	Init kinect device and kinect thread.
	@return  true if successfull, else false.
*/
bool kinectInit(void);

void kinectClose(void);
float *getDistanceMap(void);
uint8_t* getRgb(void);
void kinectUpdateData(void);
IplImage* getCvRgb(void);
int kinectGetImageWidth(int isIRModeOn);
int kinectGetImageHeight(int isIRModeOn);
IplImage* kinectGetDepthImage(void);
IplImage* kinectGetDistanceImage(void);
IplImage* kinectGetIRImage(void);
IplImage* kinectGetRGBImage(void);
int kinectSetIRMode(freenect_video_format irFormat);
int kinectSetRGBMode(void);

// FF+
int sendKinectCommand(uint16_t reg, uint16_t data, uint8_t adv_cmd);
uint16_t getKinectValue(uint16_t reg, uint8_t adv_cmd);
// /FF+

/**
	Convert raw depth value to meters.
	
	@param  raw_depth  raw depth value given by Kinect in [ 0 , 2047 ].
	@return  depth in meters.
*/
float rawDepthToMeters(int raw_depth);
