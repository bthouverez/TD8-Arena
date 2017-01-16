#include "cameraKinect.h"

#include <stdio.h>
#include <string.h>
#include <highgui.h>

#include "Common/unixtowin.h"

// the class factories

extern "C" DLL_EXPORT apicamera::Camera* create(void)
{
    return new apicamera::CameraKinect;
}

extern "C" DLL_EXPORT void destroy(apicamera::Camera* p) 
{
    delete p;
}


namespace apicamera
{

//-------------------------------------------------------------------

CameraKinect::CameraKinect()
{
	//~ printf( "ici CameraKinect::CameraKinect()\n");
	
	outputImageType = IMGTYPE_RGB;
	IRMode = 0;
	IRScaleUpOn = 0;

    undistortMapx_depth = NULL;
	undistortMapy_depth = NULL;

	for( int i = 0; i < sizeof(lastFrames)/sizeof(lastFrames[0]); i++)
		lastFrames[i] = NULL;
}

//-------------------------------------------------------------------

CameraKinect::~CameraKinect()
{
	//printf( "ici CameraKinect::~CameraKinect()\n");

	close();
}

//-------------------------------------------------------------------

int CameraKinect::open( int numCam, OpenParameters *param)
{
	if( ! kinectInit() )
	{
		fprintf( stderr, "ERROR: failed to initialize camera Kinect %d.\n", 
			numCam);
		return -1;
	}

	setParameter( PARAM_OUTPUTIMGTYPE, IMGTYPE_RGB);

	framerate = 0;
	frameFormat = 0;
	
	// initialization is ok ...
	sprintf( name, "Kinect-%d", numCam);
	fprintf( stderr, "SUCCESS: %s detected.\n", name);
	setActive(true);

	return 0;
}

//-------------------------------------------------------------------

void CameraKinect::close(void) 
{
	kinectClose();
	
	cvReleaseImage(&undistortMapx_depth);
	undistortMapx_depth = NULL;
	cvReleaseImage(&undistortMapy_depth);
	undistortMapy_depth = NULL;
	lastFrame = NULL;
	
	Camera::close();
}

//-------------------------------------------------------------------

void* CameraKinect::getRawDataPtr(void)
{
	kinectUpdateData();
	
	lastFrames[IMGTYPE_DEPTH] = kinectGetDepthImage();
	lastFrames[IMGTYPE_DISTANCE] = kinectGetDistanceImage();
	
	lastFrames[IMGTYPE_IR_10BITS] = kinectGetIRImage();
	lastFrames[IMGTYPE_IR_8BITS] = kinectGetIRImage();
	lastFrames[IMGTYPE_RGB] = kinectGetRGBImage();

	return (void*) lastFrames;
}

//-------------------------------------------------------------------

IplImage* CameraKinect::get1Frame(void)
{
	getRawDataPtr();
	
	// output image depends on outputImageType
	lastFrame = lastFrames[outputImageType];

	// scale up 10 bits IR image
	if( IRScaleUpOn && IRMode == 2 )
		cvConvertScale( lastFrames[IMGTYPE_IR_10BITS], lastFrames[IMGTYPE_IR_10BITS], 64, 0);

	if( isUndistortModeOn() )
	{
		// IR frame is NOT undistorded
		
		if( ! isIRModeOn() )
			undistortFrame(getRGB());

		undistortFrame( getDepth(), undistortMapx_depth, undistortMapy_depth);
		undistortFrame( getDistance(), undistortMapx_depth, undistortMapy_depth);
	}
		
	return lastFrame;
}

//-------------------------------------------------------------------

int CameraKinect::getParameter(int control)
{
	int iResult = -1;

	switch( control )
	{
		case PARAM_IRMODE :
			iResult = IRMode;
		break;

		case PARAM_RGBMODE :
			if( isIRModeOn() )
				iResult = 0;
			else
				iResult = 1;
		break;
		
		case PARAM_OUTPUTIMGTYPE :
			iResult = outputImageType;
		break;

		case PARAM_RGB_EXPOSURE :
			iResult = getKinectValue( 0x0007, 1);
		break;

		case PARAM_RGB_SENSITIVITY :
			iResult = getKinectValue( 0x0009, 1);
		break;
	}

	return iResult;
}

//-------------------------------------------------------------------

int CameraKinect::setParameter(int control, int value)
{
	int iResult = 0;

	switch( control )
	{
		case PARAM_IRMODE :
			if( value == 0 )
				break; // invalid value
			else if( value == 1 )
				iResult = kinectSetIRMode(FREENECT_VIDEO_IR_8BIT);
			else
				iResult = kinectSetIRMode(FREENECT_VIDEO_IR_10BIT);

			if( iResult != 0 )
			{
				fprintf( stderr, "%s : Failed to set 10 bits IR Mode.\n",
					__PRETTY_FUNCTION__);
				break;
			}

			// success
			IRMode = value;
		break;
		
		case PARAM_RGBMODE :
			if( kinectSetRGBMode() != 0 )
			{
				fprintf( stderr, "%s : Failed to set RGB Mode.\n",
					__PRETTY_FUNCTION__);
				iResult = -1;
				break;
			}

			// success
			IRMode = 0;
		break;

		case PARAM_OUTPUTIMGTYPE :
			if( value < IMGTYPE_BEGIN || value > IMGTYPE_END )
			{
				fprintf( stderr, "Value out of range. Image type is a value from %d to %d.\n", IMGTYPE_BEGIN, IMGTYPE_END);
				iResult = -1;
			}

			// change video mode if needed

			if( value == IMGTYPE_RGB )
			{
				// turn into RGB mode
				if( setParameter( PARAM_RGBMODE, 1) != 0 )
				{
					// failed
					iResult = -1;
					break;
				}
			}

			if( value == IMGTYPE_IR_8BITS )
			{
				// turn into 8 bits IR mode
				if( setParameter( PARAM_IRMODE, 1) != 0 )
				{
					// failed
					iResult = -1;
					break;
				}
			}

			if( value == IMGTYPE_IR_10BITS )
			{
				// turn into 10 bits IR mode
				if( setParameter( PARAM_IRMODE, 2) != 0 )
				{
					// failed
					iResult = -1;
					break;
				}
			}

			// now kinect is in right video mode

			outputImageType = value;
			if( value == IMGTYPE_IR_10BITS || value == IMGTYPE_IR_8BITS )
			{
				frameWidth = KINECT_IR_WIDTH;
				frameHeight = KINECT_IR_HEIGHT;
			}
			else
			{
				frameWidth = KINECT_IMG_WIDTH;
				frameHeight = KINECT_IMG_HEIGHT;
			}
		break;

		case PARAM_IRSCALEUP :
			IRScaleUpOn = value;
		break;

		case PARAM_RGB_MANUAL_PARAMETERS :
			sendKinectCommand( 0x8106, 0x848E, 1); //disable auto exposure
			sendKinectCommand( 0x8125, 0x0000, 1); //disable auto saturation
			sendKinectCommand( 0x8105, 0x0000, 1); //disable aperture correction
		break;

		case PARAM_RGB_EXPOSURE :
			iResult = sendKinectCommand( 0x8007, value, 1); 
		break;

		case PARAM_RGB_SENSITIVITY :
			iResult = sendKinectCommand( 0x8009, value, 1); 
		break;
	}

	if( iResult < 0 )
    {
        fprintf( stderr, "CameraKinect::setParameter( %d, %d) failed, with return value of %d !!\n", control, value, iResult);
        return -1;
    }

	return 0;
}

//-------------------------------------------------------------------

void CameraKinect::listParameters(void)
{
	const int bufSize = 48;
	char parameterName[bufSize];
	long minVal, maxVal;
	int step;

	printf("\nCamera parameters list :\n");
	printf( "%35s %8s %8s %8s %8s\n", "ID", "CURRENT", "MIN", "MAX", "STEP");

	for( int iPar = PARAM_FIRST+1; iPar < PARAM_LAST; iPar++)
	{
		switch( iPar )
		{
			case PARAM_IRMODE :
				minVal = 0;
				maxVal = 2;
				step = 1;
			break;

			case PARAM_RGBMODE :
				minVal = 0;
				maxVal = 1;
				step = 1;
			break;

			case PARAM_OUTPUTIMGTYPE :
				minVal = IMGTYPE_BEGIN;
				maxVal = IMGTYPE_END;
				step = 1;
			break;
		}
		
		getCameraParameterName( iPar, parameterName, bufSize);
		printf( "%35s %8d %8ld %8ld %8d\n", parameterName, getParameter(iPar), minVal, maxVal, step); 
	}
	
	printf("\n");
}

//-------------------------------------------------------------------

void CameraKinect::getCameraParameterName( int parId, char* buffer, int bufSize)
{
	switch( parId )
	{
		case PARAM_IRMODE :
			snprintf( buffer, bufSize, "%s", "PARAM_IRMODE");
		break;

		case PARAM_RGBMODE :
			snprintf( buffer, bufSize, "%s", "PARAM_RGBMODE");
		break;

		case PARAM_OUTPUTIMGTYPE :
			snprintf( buffer, bufSize, "%s", "PARAM_OUTPUTIMGTYPE");
		break;
		
		default :
			snprintf( buffer, bufSize, "%s", "UNKNOWN");
		break;
	}
}

//-------------------------------------------------------------------

void CameraKinect::loadIntrinsicParameters(const char *fileName)
{
	FILE *file;
		
	file = fopen( fileName, "r");
	if( file == NULL )
	{
		printf( "%s : failed to open %s file.\n", __PRETTY_FUNCTION__, fileName);
		return;
	}
	
	setlocale( LC_ALL, "POSIX");
	
	int result;

	// load RGB camera intrinsic parameters

	result = fscanf( file, "%*s");	// jump "distortion:"
	result =  fscanf( file, "%f %f %f %f", &(intrinsicK[0]), &(intrinsicK[1]), &(intrinsicK[2]), &(intrinsicK[3]));
	
	result =  fscanf( file, "%*s");	// jump "projection:"
	result =  fscanf( file, "%f %f %f", &(intrinsicA[0]), &(intrinsicA[1]), &(intrinsicA[2]));
	result =  fscanf( file, "%f %f %f", &(intrinsicA[3]), &(intrinsicA[4]), &(intrinsicA[5]));
	result =  fscanf( file, "%f %f %f", &(intrinsicA[6]), &(intrinsicA[7]), &(intrinsicA[8]));

	// load depth camera intrinsic parameters

	// look for "distortion:" string, which is de beginning of depth camera
	// section
	char buffer[32];
	do
	{
		result = fscanf( file, "%31s", buffer);	
	}
	while( strcmp( buffer, "distortion:") != 0  &&  result != EOF );

	if( result == EOF )
	{
		printf( "%s : failed to load depth intrinsic parameters from  %s file.\n", __PRETTY_FUNCTION__, fileName);
		return;
	}

	// read depth distortion
	result = fscanf( file, "%f %f %f %f", &(intrinsicK_depth[0]), &(intrinsicK_depth[1]), &(intrinsicK_depth[2]), &(intrinsicK_depth[3]));

	// read depth projection matrix
	result =  fscanf( file, "%*s");	// jump "projection:"
	result =  fscanf( file, "%f %f %f", &(intrinsicA_depth[0]), &(intrinsicA_depth[1]), &(intrinsicA_depth[2]));
	result =  fscanf( file, "%f %f %f", &(intrinsicA_depth[3]), &(intrinsicA_depth[4]), &(intrinsicA_depth[5]));
	result =  fscanf( file, "%f %f %f", &(intrinsicA_depth[6]), &(intrinsicA_depth[7]), &(intrinsicA_depth[8]));

	fclose(file);

	// compute undistortion parameters
	initUndistortMaps();	
}

//-------------------------------------------------------------------

void CameraKinect::initUndistortMaps(void)
{
	CvSize outputResolution;
	MAT33 halfResolA;
	MAT33 halfResolA_depth;
	float *ptrA;
	float *ptrA_depth;

	if( isDownsampleModeOn() )
	{
		// output resolution is half input resolution on 2 axis
		outputResolution = cvSize( frameWidth/2, frameHeight/2);

		// intrinsic parameters are modified accordingly
		mat33_copy( halfResolA, intrinsicA);
		halfResolA[0] = halfResolA[0] / 2.0f;
		halfResolA[2] = halfResolA[2] / 2.0f;
		halfResolA[4] = halfResolA[4] / 2.0f;
		halfResolA[5] = halfResolA[5] / 2.0f;

		ptrA = halfResolA;

		mat33_copy( halfResolA_depth, intrinsicA_depth);
		halfResolA_depth[0] = halfResolA_depth[0] / 2.0f;
		halfResolA_depth[2] = halfResolA_depth[2] / 2.0f;
		halfResolA_depth[4] = halfResolA_depth[4] / 2.0f;
		halfResolA_depth[5] = halfResolA_depth[5] / 2.0f;

		ptrA_depth = halfResolA_depth;
	}
	else
	{
		outputResolution = cvSize( frameWidth, frameHeight);
		ptrA = intrinsicA;
		ptrA_depth = intrinsicA_depth;
	}

	// allocate memory for mapx
	cvReleaseImage(&undistortMapx);
	undistortMapx = cvCreateImage( outputResolution, IPL_DEPTH_32F, 1 );
	cvReleaseImage(&undistortMapx_depth);
	undistortMapx_depth = cvCreateImage( outputResolution, IPL_DEPTH_32F, 1 );

	// allocate memory for mapy
	cvReleaseImage(&undistortMapy);
	undistortMapy = cvCreateImage( outputResolution, IPL_DEPTH_32F, 1 );
	cvReleaseImage(&undistortMapy_depth);
	undistortMapy_depth = cvCreateImage( outputResolution, IPL_DEPTH_32F, 1 );

	if( undistortMapx == NULL  ||  undistortMapx_depth == NULL
		||  undistortMapy == NULL  ||  undistortMapy_depth == NULL )
	{
		fprintf( stderr, "ERROR in %s : cvCreateImage failed.\n", __PRETTY_FUNCTION__);
		return;
	}

	// compute maps
	CvMat intrinsicMat = cvMat( 3, 3, CV_32FC1, (void*) ptrA);
	CvMat distortionCoeffs = cvMat( 1, 4, CV_32FC1, (void*) intrinsicK);
	cvInitUndistortMap( &intrinsicMat, &distortionCoeffs, undistortMapx, undistortMapy);

	CvMat intrinsicMat_depth = cvMat( 3, 3, CV_32FC1, (void*) ptrA_depth);
	CvMat distortionCoeffs_depth = cvMat( 1, 4, CV_32FC1, (void*) intrinsicK_depth);
	cvInitUndistortMap( &intrinsicMat_depth, &distortionCoeffs_depth, undistortMapx_depth, undistortMapy_depth);
}

//-------------------------------------------------------------------

void CameraKinect::loadExtrinsicParameters(const char *fileName)
{
	FILE *file;
		
	file = fopen( fileName, "r");
	if( file == NULL )
	{
		printf( "%s : failed to open %s file.\n", __PRETTY_FUNCTION__, fileName);
		return;
	}
	
	setlocale( LC_ALL, "POSIX");
	
	// load rgb camera extrinsic parameters

	int result;
	result =  fscanf( file, "%*s");	// jump "rotation:"
	result =  fscanf( file, "%f %f %f", &(extrinsicR[0]), &(extrinsicR[1]), &(extrinsicR[2]));
	result =  fscanf( file, "%f %f %f", &(extrinsicR[3]), &(extrinsicR[4]), &(extrinsicR[5]));
	result =  fscanf( file, "%f %f %f", &(extrinsicR[6]), &(extrinsicR[7]), &(extrinsicR[8]));

	result =  fscanf( file, "%*s");	// jump "translation:"
	result =  fscanf( file, "%f %f %f", &(extrinsicT[0]), &(extrinsicT[1]), &(extrinsicT[2]));

	// load depth camera extrinsic parameters

	// look for "rotation:" string, which is de beginning of depth camera
	// section
	char buffer[32];
	do
	{
		result = fscanf( file, "%31s", buffer);	
	}
	while( strcmp( buffer, "rotation:") != 0  &&  result != EOF );

	if( result == EOF )
	{
		printf( "%s : failed to load depth extrinsic parameters from  %s file.\n", __PRETTY_FUNCTION__, fileName);
		return;
	}

	// read depth camera rotation
	result =  fscanf( file, "%f %f %f", &(extrinsicR_depth[0]), &(extrinsicR_depth[1]), &(extrinsicR_depth[2]));
	result =  fscanf( file, "%f %f %f", &(extrinsicR_depth[3]), &(extrinsicR_depth[4]), &(extrinsicR_depth[5]));
	result =  fscanf( file, "%f %f %f", &(extrinsicR_depth[6]), &(extrinsicR_depth[7]), &(extrinsicR_depth[8]));

	// read depth camera translation
	result =  fscanf( file, "%*s");	// jump "translation:"
	result =  fscanf( file, "%f %f %f", &(extrinsicT_depth[0]), &(extrinsicT_depth[1]), &(extrinsicT_depth[2]));

	fclose(file);
}

//-------------------------------------------------------------------

IplImage* CameraKinect::colorCodedDepth(IplImage* depth)
{
	if( depth->nChannels != 1 )
	{
		fprintf( stderr, "%s : depth image must have 1 channel. Aborting.\n",
			__PRETTY_FUNCTION__);
		return NULL;
	}

	// convert depth image to color
	IplImage *colored = cvCreateImage( cvGetSize(depth), IPL_DEPTH_8U, 3);

	for( int l = 0; l < depth->height; l++)
	{
		for( int c = 0; c < depth->width; c++)
		{
			int depthVal = ((int) cvGet2D( depth, l, c).val[0]);// / 32;
			int pixId = (l * depth->width) + c;

			// Kinect depth image use 11 bits
			int lb = depthVal & 0xff;
			unsigned char R, G, B;
			switch( depthVal >> 8 ) 
			{
				case 0:
					B = 255;
					G = 255-lb;
					R = 255-lb;
					break;
				case 1:
					B = 255;
					G = lb;
					R = 0;
					break;
				case 2:
					B = 255-lb;
					G = 255;
					R = 0;
					break;
				case 3:
					B = 0;
					G = 255;
					R = lb;
					break;
				case 4:
					B = 0;
					G = 255-lb;
					R = 255;
					break;
				case 5:
					B = 0;
					G = 0;
					R = 255-lb;
					break;
				default:
					B = 0;
					G = 0;
					R = 0;
					break;
			}

			// this works only with 8 bits per channel image
			colored->imageData[pixId*3 + 0] = B;
			colored->imageData[pixId*3 + 1] = G;
			colored->imageData[pixId*3 + 2] = R;
		}
	}

	return colored;
}

//-------------------------------------------------------------------

void CameraKinect::recordFrame(void)
{
	static int recFrameCnt = 0;
	static char bufferCnt[32]; 

	recFrameCnt++;
	std::string fileName(getName());

	// build RGB or IR file name
	if( isIRModeOn() )
	{
		// build IR file name
		snprintf( bufferCnt, sizeof(bufferCnt), "-ir-%06d", recFrameCnt);
		fileName.append(bufferCnt);
		fileName.append(".pgm");
	}
	else
	{
		// build RGB file name
		snprintf( bufferCnt, sizeof(bufferCnt), "-rgb-%06d", recFrameCnt);
		fileName.append(bufferCnt);
		fileName.append(".png");
	}
	
	// write RGB/IR image to file
	cvSaveImage( fileName.c_str(), lastFrame);
	
	// build DEPTH file name
	fileName.assign(getName());
	snprintf( bufferCnt, sizeof(bufferCnt), "-depth-%06d", recFrameCnt);
	fileName.append(bufferCnt);
	fileName.append(".pgm");
	
	// write DEPTH image to file
	cvSaveImage( fileName.c_str(), getDepth());
}

//-------------------------------------------------------------------

} // namespace apicamera
