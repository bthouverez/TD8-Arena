#include "camera.h"

#include <stdio.h>
#include <string.h>
#include <sstream>
#include <highgui.h>
#include <libconfig.h>

#ifdef D_API_WIN32
	// unix to windows porting
	#define   __PRETTY_FUNCTION__   __FUNCTION__ 
	#define  snprintf  _snprintf
#endif

namespace apicamera
{

//-------------------------------------------------------------------

Camera::Camera()
{
#ifdef _DEBUG
	printf( "entering Camera::%s ...\n", __func__);
#endif

	strcpy( name, "Not initialised");
	model.assign("undefined");
	
	frameWidth = 0;
	frameHeight = 0;
	
	fDownsample = false;
	fUndistort = false;

	lastFrame = NULL;	
	undistortMapx = NULL;
	undistortMapy = NULL;
}

//-------------------------------------------------------------------

Camera::~Camera()
{
#ifdef _DEBUG
	printf( "entering Camera::%s ...\n", __func__);
#endif
	close();
}

//-------------------------------------------------------------------

int Camera::open( int numCam, const char *fileName)
{
	OpenParameters parameters;

	if( loadOpenParametersFromFile( &parameters, fileName) != 0 )
		return -1; // failed to load parameters

#ifdef _DEBUG
	// display parameters
	printf("Parameters value :\n");
	displayOpenParameters(&parameters);
	printf("\n");
#endif

	// initialize camera
	int res = open( numCam, &parameters);

	if( res == 0 )
		setParameter(setParameterString);

	return res;
}

//-------------------------------------------------------------------

int Camera::open( int numCam, unsigned int width, unsigned int height,
	unsigned int fRate, unsigned int format)
{
	OpenParameters parameters;
	
	parameters.width = width;
	parameters.height = height;
	parameters.fRate = fRate;
	
	return open( numCam, &parameters);
}

//-------------------------------------------------------------------

void Camera::close(void)
{
#ifdef _DEBUG
	printf( "entering Camera::%s ...\n", __func__);
#endif

	// release memory
	if( undistortMapx != NULL )
		cvReleaseImage(&undistortMapx);
	if( undistortMapy != NULL )
		cvReleaseImage(&undistortMapy);
	if( lastFrame != NULL )
		cvReleaseImage(&lastFrame);
		
	lastFrame = NULL;	
	undistortMapx = NULL;
	undistortMapy = NULL;
	fDownsample = false;
	fUndistort = false;
}

//-------------------------------------------------------------------

void Camera::show(bool recordingOn)
{
	IplImage *frame = NULL;

	cvNamedWindow( name, CV_WINDOW_AUTOSIZE);

	// Show the image captured from the camera in the window and repeat
	while( 1 )
	{
		// Get one frame
		frame = get1Frame();
		if( !frame )
		{
			fprintf( stderr, "ERROR: frame is null...\n" );
			break;
		}

		if( recordingOn )
			recordFrame();

		cvShowImage( name, frame);

		//If ESC key pressed, Key=0x10001B under OpenCV 0.9.7(linux version),
		//remove higher bits using AND operator
		if( (cvWaitKey(10) & 255) == 27 ) 
			break;
	}

	// Release memory
	cvDestroyWindow(name);
}

//-------------------------------------------------------------------

void Camera::saveIntrinsicParameters(const char *fileName)
{
	FILE *file;
	
	file = fopen( fileName, "w");
	if( file == NULL )
	{
		printf( "Camera::saveIntrinsicParameters : failed to open %s file.\n", fileName);
		return;
	}
	
	setlocale( LC_ALL, "POSIX");

	fprintf( file, "distortion:\n");
	fprintf( file, "%f %f %f %f\n", intrinsicK[0], intrinsicK[1], intrinsicK[2], intrinsicK[3]);

	fprintf( file, "projection:\n");
	fprintf( file, "%f %f %f\n", intrinsicA[0], intrinsicA[1], intrinsicA[2]);  
	fprintf( file, "%f %f %f\n", intrinsicA[3], intrinsicA[4], intrinsicA[5]);
	fprintf( file, "%f %f %f\n", intrinsicA[6], intrinsicA[7], intrinsicA[8]);

	fprintf( file, "error:\n");
	fprintf( file, "%f\n", intrinsicError);
	
	fclose(file);
}

//-------------------------------------------------------------------

void Camera::loadIntrinsicParameters(const char *fileName)
{
	FILE *file;
		
	file = fopen( fileName, "r");
	if( file == NULL )
	{
		printf( "Camera::loadIntrinsicParameters : failed to open %s file.\n", fileName);
		return;
	}
	
	setlocale( LC_ALL, "POSIX");
	
	int result;
	result = fscanf( file, "%*s");	// jump "distortion:"
	result =  fscanf( file, "%f %f %f %f", &(intrinsicK[0]), &(intrinsicK[1]), &(intrinsicK[2]), &(intrinsicK[3]));
	
	result =  fscanf( file, "%*s");	// jump "projection:"
	result =  fscanf( file, "%f %f %f", &(intrinsicA[0]), &(intrinsicA[1]), &(intrinsicA[2]));
	result =  fscanf( file, "%f %f %f", &(intrinsicA[3]), &(intrinsicA[4]), &(intrinsicA[5]));
	result =  fscanf( file, "%f %f %f", &(intrinsicA[6]), &(intrinsicA[7]), &(intrinsicA[8]));

	fclose(file);

	// compute undistortion parameters
	initUndistortMaps();	
}

//-------------------------------------------------------------------

void Camera::saveExtrinsicParameters(const char *fileName)
{
	FILE *file;
	
	file = fopen( fileName, "w");
	if( file == NULL )
	{
		printf( "Calibration::saveExtrinsicParameters : failed to open %s file.\n", fileName);
		return;
	}

	setlocale( LC_ALL, "POSIX");

	fprintf( file, "rotation:\n");
	fprintf( file, "%f %f %f\n", extrinsicR[0], extrinsicR[1], extrinsicR[2]);
	fprintf( file, "%f %f %f\n", extrinsicR[3], extrinsicR[4], extrinsicR[5]);
	fprintf( file, "%f %f %f\n", extrinsicR[6], extrinsicR[7], extrinsicR[8]);

	fprintf( file, "translation:\n");
	fprintf( file, "%f %f %f\n", extrinsicT[0], extrinsicT[1], extrinsicT[2]);

	fprintf( file, "error:\n");
	fprintf( file, "%f\n", extrinsicError);
	
	fclose(file);
}

//-------------------------------------------------------------------

void Camera::loadExtrinsicParameters(const char *fileName)
{
	FILE *file;
		
	file = fopen( fileName, "r");
	if( file == NULL )
	{
		printf( "%s : failed to open %s file.\n", fileName, __PRETTY_FUNCTION__);
		return;
	}
	
	setlocale( LC_ALL, "POSIX");
	
	int result;
	result =  fscanf( file, "%*s");	// jump "rotation:"
	result =  fscanf( file, "%f %f %f", &(extrinsicR[0]), &(extrinsicR[1]), &(extrinsicR[2]));
	result =  fscanf( file, "%f %f %f", &(extrinsicR[3]), &(extrinsicR[4]), &(extrinsicR[5]));
	result =  fscanf( file, "%f %f %f", &(extrinsicR[6]), &(extrinsicR[7]), &(extrinsicR[8]));

	result =  fscanf( file, "%*s");	// jump "translation:"
	result =  fscanf( file, "%f %f %f", &(extrinsicT[0]), &(extrinsicT[1]), &(extrinsicT[2]));

	fclose(file);
}

//-------------------------------------------------------------------

void Camera::initUndistortMaps(void)
{
	CvSize outputResolution;
	MAT33 halfResolA;
	float *ptrA;
  
  	if( frameWidth == 0 || frameHeight == 0 )
  		return; // bad frame size, do nothing
  		
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
    }
    else
    {
    	outputResolution = cvSize( frameWidth, frameHeight);
    	ptrA = intrinsicA;
	}

	// allocate memory for mapx
	if( undistortMapx != NULL )
		cvReleaseImage(&undistortMapx);
	
	undistortMapx = cvCreateImage( outputResolution, IPL_DEPTH_32F, 1 );
    
    if( undistortMapx == NULL )
    {
        fprintf( stderr, "ERROR in Camera::initUndistortMaps : cvCreateImage failed.\n");
		return;
	}

	// allocate memory for mapy
	if( undistortMapy != NULL )
		cvReleaseImage(&undistortMapy);
		
	undistortMapy = cvCreateImage( outputResolution, IPL_DEPTH_32F, 1 );
	
    if( undistortMapy == NULL )
    {
        fprintf( stderr, "ERROR in Camera::initUndistortMaps : cvCreateImage failed.\n");
		return;
	}

	// compute maps
	CvMat intrinsicMat = cvMat( 3, 3, CV_32FC1, (void*) ptrA);
	CvMat distortionCoeffs = cvMat( 1, 4, CV_32FC1, (void*) intrinsicK);
	cvInitUndistortMap( &intrinsicMat, &distortionCoeffs, undistortMapx, undistortMapy);
}

//-------------------------------------------------------------------

IplImage* Camera::undistortFrame( IplImage* frame, const CvArr* mapx, const CvArr* mapy)
{
	if( mapx == NULL )
		mapx = undistortMapx;

	if( mapy == NULL )
		mapy = undistortMapy;

	IplImage* tmpImg = cvCloneImage(frame);
	cvRemap( tmpImg, frame, mapx, mapy);//, int flags=CV_INTER_LINEAR+CV_WARP_FILL_OUTLIERS, CvScalar fillval=cvScalarAll(0))
	cvReleaseImage(&tmpImg);
	
	return frame;	
}

//-------------------------------------------------------------------

int Camera::setUndistortMode(bool state)
{
	fUndistort = state;
	
	return 0;	// successfull
}	

//-------------------------------------------------------------------

void Camera::displayOpenParameters(const OpenParameters* param)
{
	printf( "width = %u\n", param->width);
	printf( "height = %u\n", param->height);	
	printf( "fRate = %u\n", param->fRate);
	//printf("\n");	
	printf( "UVCformat = %u\n", param->UVCformat);
	//printf("\n");	
	printf( "IEEEformat = %u\n", param->IEEEformat);
	//printf("\n");	
	printf( "FILEFileNamePattern = %s\n", param->FILEFileNamePattern.c_str());
	printf( "FILEIndexMin = %u\n", param->FILEIndexMin);
	printf( "FILEIndexMax = %u\n", param->FILEIndexMax);
	//printf("\n");	
	printf( "SWRmode = %d\n", param->SWRmode);
	printf( "SWRmodulationFrq = %d\n", param->SWRmodulationFrq);
}

//-------------------------------------------------------------------

int config_lookup_int32( const config_t *config, const char *path,
	int *value)
{
#ifdef FIX_CONFIG_LOOKUP_INT_132_2
	// ubuntu 10.10 & 12.04
	// config_lookup_int() in libconfig 1.3.2-2 is bugged
	long lvalue;

	if( ! config_lookup_int( config, path, &lvalue) )
		return CONFIG_FALSE;
		
	*value = (int) lvalue;
	
	return CONFIG_TRUE;
#else
	// on win, libconfig 1.4.7 is ok
	// on ubuntu 14.04, libconfig 1.4.9-2 is ok
	return config_lookup_int( config, path, value);
#endif	
}
  
//-------------------------------------------------------------------

int Camera::loadOpenParametersFromFile( OpenParameters* param, const char *fileName)
{
	config_t cfg;
	
	// load configuration file
	config_init(&cfg);
	if( ! config_read_file( &cfg, fileName)) 
	{
		fprintf( stderr, "Error line %d in configuration file %s : %s\n",
			config_error_line(&cfg),
			fileName,
			config_error_text(&cfg));
		config_destroy(&cfg);
		
		return -1; // failed
	}

	// load parameters
	const char *str;
	config_lookup_int32( &cfg, "width", (int*) &(param->width));
	config_lookup_int32( &cfg, "height", (int*) &(param->height));
	config_lookup_int32( &cfg, "fRate", (int*) &(param->fRate));
	config_lookup_int32( &cfg, "UVCformat", (int*) &(param->UVCformat));
	config_lookup_int32( &cfg, "IEEEformat", (int*) &(param->IEEEformat));
	config_lookup_string( &cfg, "FILEFileNamePattern", &str);
	param->FILEFileNamePattern.assign(str);
	config_lookup_int32( &cfg, "FILEIndexMin", (int*) &(param->FILEIndexMin));
	config_lookup_int32( &cfg, "FILEIndexMax", (int*) &(param->FILEIndexMax));
	config_lookup_int32( &cfg, "SWRmode", &(param->SWRmode));
	config_lookup_int32( &cfg, "SWRmodulationFrq", 
		&(param->SWRmodulationFrq));
	config_lookup_string( &cfg, "NETSrvIP", &str);
	param->NETSrvIP.assign(str);
	config_lookup_int32( &cfg, "NETPort", &(param->NETPort));
	config_lookup_int32( &cfg, "NETBgSubCtrl", &(param->NETBgSubCtrl));

	config_lookup_string( &cfg, "setparameter", &str);
	setParameterString.assign(str);

	// close configuration file
	config_destroy(&cfg);
	
	return 0; // ok
}

//-------------------------------------------------------------------

void Camera::recordFrame(void)
{
	static int recFrameCnt = 0;
	static char bufferCnt[16]; 
	
	// build file name
	recFrameCnt++;
	std::string fileName(getName());
	snprintf( bufferCnt, sizeof(bufferCnt), "-%06d", recFrameCnt);
	fileName.append(bufferCnt);
	fileName.append(".png");
	
	// write image to file
	cvSaveImage( fileName.c_str(), lastFrame);
}
  
//-------------------------------------------------------------------

void Camera::setParameter( const std::string& params)
{
	int id, value;
	std::stringstream paramStream(params);

	while( ! paramStream.eof() )
	{
		paramStream >> id;
		paramStream >> value;

		if( paramStream.fail() )
			break;

		setParameter( id, value);
	}
}

//-------------------------------------------------------------------

} // namespace Apicamera

