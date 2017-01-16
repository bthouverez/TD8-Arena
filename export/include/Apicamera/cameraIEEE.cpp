#include "cameraIEEE.h"

#include <stdio.h>
#include <string.h>
#include <math.h>


// the class factories

extern "C" DLL_EXPORT apicamera::Camera* create(void)
{
    return new apicamera::CameraIEEE;
}

extern "C" DLL_EXPORT void destroy(apicamera::Camera* p) 
{
    delete p;
}


namespace apicamera
{

//-------------------------------------------------------------------

CameraIEEE::CameraIEEE()
{
	//printf( "ici CameraIEEE::CameraIEEE()\n");
	dc1394Camera = NULL;
	dc1394Context = NULL;
	rawDataBuffer = NULL;
}

//-------------------------------------------------------------------

CameraIEEE::~CameraIEEE()
{
	//printf( "ici CameraIEEE::~CameraIEEE()\n");
	close();
}

//-------------------------------------------------------------------

int CameraIEEE::open( int numCam, unsigned int width, unsigned int height, unsigned int fRate, unsigned int format)
{
	dc1394camera_list_t  *list;
	dc1394error_t  err;
	
	// initialize IEEE1394
	dc1394Context = dc1394_new();
	if ( ! dc1394Context )
	{
		fprintf( stderr, "ERROR: Can't open IEEE1394 context.\n");
		return -1;
	}
	
	// detect IEEE1394 camera	
	err = dc1394_camera_enumerate( dc1394Context, &list);
	//DC1394_ERR_RTN( err, "Failed to enumerate cameras");
	if ( list->num == 0 )
	{
		fprintf( stderr, "ERROR: Failed to enumerate IEEE1394 cameras.\n");
		close();
		return -1;
	}

	dc1394Camera = NULL;
	//printf("debug : list->num = %d\n", list->num);
	if ( numCam+1 <= list->num )
		dc1394Camera = dc1394_camera_new( dc1394Context, list->ids[numCam].guid);
	if ( ! dc1394Camera )
	{
		fprintf( stderr, "WARNING: Failed to initialize IEEEcamera %d.\n", numCam);
		dc1394_camera_free_list( list );
		close();
		return -1;
	}
	dc1394_camera_free_list( list );

	// configure IEEE1394 camera
	
	// set resolution and colour coding
	if( setResolutionAndFormat( width, height, format) != 0 )
	{
		fprintf( stderr, "ERROR: Failed to set video mode (%u,%u) for IEEE camera %d.\n", width, height, numCam);
		close();
		return -1;
	}
	frameWidth = width;
	frameHeight = height;

	// set framerate
	if ( setFramerate(fRate) != 0 )
	{
		fprintf( stderr, "ERROR: Failed to set framerate to %u fps for IEEE camera %d.\n", fRate, numCam);
		close();
		return -1;
	}
	framerate = fRate;

	if ( dc1394_capture_setup( dc1394Camera, 4, DC1394_CAPTURE_FLAGS_DEFAULT) != DC1394_SUCCESS )
	{
		fprintf( stderr, "ERROR: Failed to set setup capture for IEEE camera %d.\n", numCam);
		close();
		return -1;
	}

	// start sending data
	if ( dc1394_video_set_transmission( dc1394Camera, DC1394_ON) != DC1394_SUCCESS )
	{
		fprintf( stderr, "ERROR: Failed to set transmission on for IEEE camera %d.\n", numCam);
		close();
		return -1;
	}

    // memory allocation for BGR image !!
    lastFrame = cvCreateImage( cvSize( frameWidth, frameHeight), IPL_DEPTH_8U, 3);
    if( lastFrame == NULL )
    {
        fprintf( stderr, "ERROR: IEEE camera %d : cvCreateImage failed.\n", numCam);
		return -1;
	}

    // memory allocation for raw data buffer !!
    rawDataBuffer = (unsigned char*) malloc( frameWidth*frameHeight);
    if( rawDataBuffer == NULL )
    {
        fprintf( stderr, "ERROR: IEEE camera %d : memory allocation for raw data buffer failed.\n", numCam);
		return -1;
	}

    // si tout c'est bien passe ...
	sprintf( name, "IEEE-%d", numCam);
	fprintf( stderr, "SUCCESS: %s initialised (%ux%u pixels, %u fps).\n", name, frameWidth, frameHeight, framerate);
	setActive(true);

	return 0;
}

//-------------------------------------------------------------------

int CameraIEEE::open( int numCam, OpenParameters *param)
{
	return open( numCam, param->width, param->height, param->fRate, param->IEEEformat);		
}

//-------------------------------------------------------------------

void CameraIEEE::close(void)
{
	// IEEE1394 cleaning
	if( dc1394Camera != NULL )
	{
		dc1394_video_set_transmission( dc1394Camera, DC1394_OFF);
		dc1394_capture_stop( dc1394Camera );
		dc1394_camera_free( dc1394Camera );
		dc1394Camera = NULL;
	}
	
	if( dc1394Context != NULL )
	{
		dc1394_free( dc1394Context );
		dc1394Context = NULL;
	}
	
	// frees memory
	free(rawDataBuffer);
	rawDataBuffer = NULL;
	
	Camera::close();
}

//-------------------------------------------------------------------

void* CameraIEEE::getRawDataPtr(void)
{
	dc1394video_frame_t  *frame = NULL;
	
	/*
	if ( dc1394_capture_dequeue( dc1394Camera, DC1394_CAPTURE_POLICY_WAIT, &frame) != DC1394_SUCCESS )
	{
		fprintf( stderr, "%s : CameraIEEE::getRawDataPtr failed.\n", name);
		return NULL;
	}
	*/

	// empty dma queue
	dc1394_capture_dequeue( dc1394Camera, DC1394_CAPTURE_POLICY_POLL, &frame);
	while( frame )
	{
		memcpy( rawDataBuffer, frame->image, frameWidth*frameHeight);
		dc1394_capture_enqueue( dc1394Camera, frame);
		dc1394_capture_dequeue( dc1394Camera, DC1394_CAPTURE_POLICY_POLL, &frame);
	}

	return (void*) rawDataBuffer;
}

//-------------------------------------------------------------------

IplImage* CameraIEEE::get1Frame(void)
{
	unsigned char *rawData = NULL; 
	IplImage* bayerGRImg;

	rawData = (unsigned char *) getRawDataPtr();

	if( rawData == NULL )
	{
        fprintf( stderr, "ERROR in CameraIEEE::get1Frame : failed to get new frame.\n");
        return lastFrame;
	}	


	bayerGRImg = cvCreateImage( cvSize( frameWidth, frameHeight), IPL_DEPTH_8U, 1);
	if( bayerGRImg == NULL )
	{
	    fprintf( stderr, "ERROR: CameraIEEE::get1Frame: cvCreateImage failed for %s.\n", name);
		return lastFrame;
	}

	memcpy( (unsigned char *) bayerGRImg->imageData, rawData, bayerGRImg->imageSize);

	if( ! isDownsampleModeOn() )
		cvCvtColor( bayerGRImg, lastFrame, CV_BayerGR2BGR);
	else
		bayer_downsampleBGR( bayerGRImg, lastFrame);

	cvReleaseImage( &bayerGRImg );


	if( isUndistortModeOn() )
		undistortFrame(lastFrame);
	
	return lastFrame;
}

//-------------------------------------------------------------------

int CameraIEEE::getParameter(int control)
{
	// extended features
	if( control == DC1394_MYFEATURE_ISO_SPEED )
	{
		dc1394speed_t  speed;

		if( dc1394_video_get_iso_speed( dc1394Camera, &speed) == DC1394_SUCCESS )
			return (int) speed;
	}
	else if( control == DC1394_MYFEATURE_WHITE_BALANCE_UB_VALUE )
	{
		int UBValue, VRValue;

		if( dc1394_feature_whitebalance_get_value( dc1394Camera, (uint32_t*) &UBValue, (uint32_t*) &VRValue) == DC1394_SUCCESS )
			return UBValue;
	}
	else if( control == DC1394_MYFEATURE_WHITE_BALANCE_VR_VALUE )
	{
		int UBValue, VRValue;
		
		if( dc1394_feature_whitebalance_get_value( dc1394Camera, (uint32_t*) &UBValue, (uint32_t*) &VRValue) == DC1394_SUCCESS )
			return VRValue;
	}
	else if( control == DC1394_MYFEATURE_WHITE_SHADING_R_VALUE )
	{
		int RValue, GValue, BValue;
		
		if( dc1394_feature_whiteshading_get_value( dc1394Camera, (uint32_t*) &RValue, (uint32_t*) &GValue, (uint32_t*) &BValue) == DC1394_SUCCESS )
			return RValue;
	}
	else if( control == DC1394_MYFEATURE_WHITE_SHADING_G_VALUE )
	{
		int RValue, GValue, BValue;
		
		if( dc1394_feature_whiteshading_get_value( dc1394Camera, (uint32_t*) &RValue, (uint32_t*) &GValue, (uint32_t*) &BValue) == DC1394_SUCCESS )
			return GValue;
	}
	else if( control == DC1394_MYFEATURE_WHITE_SHADING_B_VALUE )
	{
		int RValue, GValue, BValue;
		
		if( dc1394_feature_whiteshading_get_value( dc1394Camera, (uint32_t*) &RValue, (uint32_t*) &GValue, (uint32_t*) &BValue) == DC1394_SUCCESS )
			return BValue;
	}
	// other features
	else
	{
		dc1394feature_info_t  feature;
	
		feature.id = (dc1394feature_t) control;
	
		if( dc1394_feature_get( dc1394Camera, &feature) == DC1394_SUCCESS )
			return feature.value;
	}
	
	
	fprintf( stderr, "%s : CameraIEEE::getParameter(%d) failed.\n", name, control);
	return -1;
}

//-------------------------------------------------------------------

int CameraIEEE::setParameter( int control, int value)
{
	// extended features
	if( control == DC1394_MYFEATURE_ISO_SPEED  &&  dc1394_video_set_iso_speed( dc1394Camera, (dc1394speed_t) value) == DC1394_SUCCESS )
		return 0;
	else if( control == DC1394_MYFEATURE_WHITE_BALANCE_UB_VALUE )
	{
		int VRValue;
		
		VRValue = getParameter(DC1394_MYFEATURE_WHITE_BALANCE_VR_VALUE);
		
		if( dc1394_feature_whitebalance_set_value( dc1394Camera, value, VRValue) == DC1394_SUCCESS )
			return 0;
	}
	else if( control == DC1394_MYFEATURE_WHITE_BALANCE_VR_VALUE )
	{
		int UBValue;
		
		UBValue = getParameter(DC1394_MYFEATURE_WHITE_BALANCE_UB_VALUE);
		
		if( dc1394_feature_whitebalance_set_value( dc1394Camera, UBValue, value) == DC1394_SUCCESS )
			return 0;
	}
	else if( control == DC1394_MYFEATURE_WHITE_SHADING_R_VALUE )
	{
		int GValue, BValue;
		
		GValue = getParameter(DC1394_MYFEATURE_WHITE_SHADING_G_VALUE);		
		BValue = getParameter(DC1394_MYFEATURE_WHITE_SHADING_B_VALUE);
		
		if( dc1394_feature_whiteshading_set_value(dc1394Camera, value, GValue, BValue) == DC1394_SUCCESS )
			return 0;
	}
	else if( control == DC1394_MYFEATURE_WHITE_SHADING_G_VALUE )
	{
		int RValue, BValue;
		
		RValue = getParameter(DC1394_MYFEATURE_WHITE_SHADING_R_VALUE);		
		BValue = getParameter(DC1394_MYFEATURE_WHITE_SHADING_B_VALUE);
		
		if( dc1394_feature_whiteshading_set_value(dc1394Camera, RValue, value, BValue) == DC1394_SUCCESS )
			return 0;
	}
	else if( control == DC1394_MYFEATURE_WHITE_SHADING_B_VALUE )
	{
		int RValue, GValue;
		
		RValue = getParameter(DC1394_MYFEATURE_WHITE_SHADING_R_VALUE);		
		GValue = getParameter(DC1394_MYFEATURE_WHITE_SHADING_G_VALUE);
		
		if( dc1394_feature_whiteshading_set_value(dc1394Camera, RValue, GValue, value) == DC1394_SUCCESS )
			return 0;
	}
	// other features
	else if( dc1394_feature_set_value( dc1394Camera, (dc1394feature_t) control, (uint32_t) value) == DC1394_SUCCESS )
		return 0;


	fprintf( stderr, "%s : CameraIEEE::setParameter(%d,%d) failed.\n", name, control, value);
	return -1;	
}

//-------------------------------------------------------------------

void CameraIEEE::listParameters(void)
{
	dc1394featureset_t  features;

	if( dc1394_feature_get_all( dc1394Camera, &features) != DC1394_SUCCESS )
		fprintf( stderr, "%s : CameraIEEE::listParameters failed.\n", name);
	else
		dc1394_feature_print_all( &features, stdout);
}

//-------------------------------------------------------------------

int CameraIEEE::setResolutionAndFormat( unsigned int width, unsigned int height, unsigned int format)
{
	bool bVideoModeFound = false;
	int mode;

	// search which dc1394 video mode matches the wanted resolution
	// in colour coding format MONO8 
	format = DC1394_COLOR_CODING_MONO8;
	for( mode = DC1394_VIDEO_MODE_MIN; mode <= DC1394_VIDEO_MODE_MAX; mode++)
	{
		dc1394color_coding_t  colorCoding;
		unsigned int modeWidth = 0, modeHeight = 0;
		
		dc1394_get_color_coding_from_video_mode( dc1394Camera, (dc1394video_mode_t) mode, &colorCoding); 
		dc1394_get_image_size_from_video_mode( dc1394Camera, mode, &modeWidth, &modeHeight);
		if( colorCoding  == format  &&  modeWidth == width  &&  modeHeight == height )
		{
			bVideoModeFound = true;
			break;
		}
	}
	
	if( ! bVideoModeFound )
		return -1;		// 0 = OK, -1 = ERROR
	
	// set video mode (set resolution and coulour coding format)
	if( dc1394_video_set_mode( dc1394Camera, (dc1394video_mode_t) mode) != DC1394_SUCCESS )
		return -1;		// 0 = OK, -1 = ERROR
	
	return 0;		// 0 = OK, -1 = ERROR
}

//-------------------------------------------------------------------

int CameraIEEE::setFramerate( unsigned int fRate)
{
	bool bFramerateFound = false;
	int mode;
	

	// search which dc1394 framerate matches the wanted one
	for( mode = DC1394_FRAMERATE_MIN; mode <= DC1394_FRAMERATE_MAX; mode++)
	{
		float modeFRate;
	
		dc1394_framerate_as_float( (dc1394framerate_t) mode, &modeFRate);
		if( fRate == floor(modeFRate) )
		{	
			bFramerateFound = true;
			break;
		}
	}	
	
	if( ! bFramerateFound )
		return -1;		// 0 = OK, -1 = ERROR
	
	// set framerate
	if( dc1394_video_set_framerate( dc1394Camera, (dc1394framerate_t) mode) != DC1394_SUCCESS )
		return -1;		// 0 = OK, -1 = ERROR
	
	return 0;		// 0 = OK, -1 = ERROR
}

//-------------------------------------------------------------------

int CameraIEEE::setDownsampleMode(bool state)
{
	if( state == isDownsampleModeOn() )
		return 0;
		// already in the right mode, nothing to do

	// resize image	storage, allocate memory for BGR image
    if( lastFrame != NULL )
    	cvReleaseImage(&lastFrame);
    
    CvSize outputResolution;
    if( state )
    	outputResolution = cvSize( frameWidth/2, frameHeight/2);
    else
    	outputResolution = cvSize( frameWidth, frameHeight);
    
	lastFrame = cvCreateImage( outputResolution, IPL_DEPTH_8U, 3);
    if( lastFrame == NULL )
    {
        fprintf( stderr, "ERROR in CameraIEEE::setDownsampleMode : cvCreateImage failed.\n");
		return -1;
	}
	
	fDownsample = state;
	
	// modify undistortion maps because resolution changed
	initUndistortMaps();

	return 0;
}

//-------------------------------------------------------------------

void CameraIEEE::bayer_downsampleBGR( const IplImage* buffer, IplImage* bgr)
{
        const unsigned int w = buffer->width, h = buffer->height;
        const unsigned int BAYER_bayer_end_img_step = (w) * (h);
        const unsigned int BAYER_bayer_end_row_step = (w);
        const unsigned int BAYER_bayer_row_end_step = (w);
        const unsigned int BAYER_bayer_row_step = 2 * (w);
        const unsigned int BAYER_bayer_col_step = 2;
        const unsigned int BAYER_bgr_col_step = 3;
        /* iterateur sur l'accumulateur */
        unsigned char *dbgr=(unsigned char*) ( bgr->imageData );
        unsigned char* bayer = (unsigned char*) ( buffer->imageData );
        /* iterateur sur le bayer */
        const unsigned char* lbayer0 = bayer + BAYER_bayer_end_img_step;
        const unsigned char* lbayer1 = bayer + BAYER_bayer_end_row_step;
        const unsigned char* bbayer = bayer;
        for ( ; bbayer < lbayer0 ; ) {
                for ( ; bbayer < lbayer1 ; ) {
                        dbgr[0]=bbayer [ 1 ];
                        dbgr[1]=bbayer [ 0 ];
                        dbgr[2]=bbayer [ w ];
                        dbgr += BAYER_bgr_col_step;
                        bbayer += BAYER_bayer_col_step;
                }
                bbayer += BAYER_bayer_row_end_step;
                lbayer1 += BAYER_bayer_row_step;
        }
}

//-------------------------------------------------------------------

} // namespace apicamera
