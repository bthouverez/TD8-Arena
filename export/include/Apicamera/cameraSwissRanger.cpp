#include <stdio.h>
#include <string.h>
#include <highgui.h>

#include "cameraSwissRanger.h"


// the class factories

extern "C" DLL_EXPORT apicamera::Camera* create(void)
{
    return new apicamera::CameraSwissRanger;
}

extern "C" DLL_EXPORT void destroy(apicamera::Camera* p) 
{
    delete p;
}


namespace apicamera
{

//-------------------------------------------------------------------

CameraSwissRanger::CameraSwissRanger()
{
	//~ printf( "ici CameraSwissRanger::CameraSwissRanger()\n");
	
	m_pSwissRangerCapture = NULL;
	m_pImgEntryArray = NULL;
	m_pSR4000Dist = NULL;
	m_pSR4000Conf = NULL;
	m_pSR4000Ampl = NULL;
	m_pSR4000AmplBack = NULL;
	m_pXYZCoords = NULL;
	outputImageType = IMGTYPE_AMPLITUDE;
	imgCnt = 0;
	
	autoExposure.minIntTime = 1;
	autoExposure.maxIntTime = 150;
	autoExposure.percentOverPos = 5;
	autoExposure.desiredPos = 70;
	
	SR_SetCallback( LibusbCallback );
}

//-------------------------------------------------------------------

CameraSwissRanger::~CameraSwissRanger()
{
	//printf( "ici CameraSwissRanger::~CameraSwissRanger()\n");
	close();
}

//-------------------------------------------------------------------

int CameraSwissRanger::open( int numCam, OpenParameters *param)
{
	int iResult = 0;

	iResult = SR_OpenUSB( &m_pSwissRangerCapture, numCam);
	if( iResult < 0 )
	{
		fprintf( stderr, "WARNING: camera SwissRanger %d not detected.\n", numCam);
		return -1;
	}
	
	iResult = setParameter( PARAM_ACQUIREMODE, param->SWRmode);
	if( iResult < 0 )
	{
		fprintf( stderr, "WARNING: failed to set acquire mode for SwissRanger %d.\n", numCam);
		return -1;
	}

	iResult = setParameter( PARAM_MODULATIONFREQUENCY, 
		(ModulationFrq) param->SWRmodulationFrq);
	if( iResult < 0 )
	{
		fprintf( stderr, "WARNING: failed to set modulation frequency for SwissRanger %d.\n", numCam);
		return -1;
	}

	iResult = SR_Acquire(m_pSwissRangerCapture);
	if( iResult < 0 )
	{
		fprintf( stderr, "WARNING: SR_Acquire failed for SwissRanger %d.\n", numCam);
		return -1;
	}
	//~ assert( iResult >= 0 );

	imgCnt = SR_GetImageList( m_pSwissRangerCapture, &m_pImgEntryArray);
	if( imgCnt < 0 )
	{
		fprintf( stderr, "WARNING: SR_GetImageList failed for SwissRanger %d.\n", numCam);
		return -1;
	}
	if( imgCnt > 3 )
	{
		// SR_GetImageList is supposed to return value 3 or 2 : distance image, amplitude image, and possibly (depending on acquire mode) confidence image
		fprintf( stderr, "ERROR: SR_GetImageList return a value > 3 for SwissRanger %d.\n", numCam);
		return -1;
	}


	frameWidth = SR_GetCols( m_pSwissRangerCapture );
	frameHeight = SR_GetRows( m_pSwissRangerCapture );
	framerate = 0;
	frameFormat = 0;
	
	m_pFrequency = (ModulationFrq) param->SWRmodulationFrq;
	
	m_pXYZCoords = new float[ frameWidth * frameHeight * 3 ];
	if( m_pXYZCoords == NULL )
	{
		fprintf( stderr, "ERROR: SwissRanger %d : memory allocation for m_pXYZCoords failed.\n", numCam);
		return -1;
	}
		
	m_pSR4000Dist = cvCreateImage( cvSize(frameWidth,frameHeight), IPL_DEPTH_8U, 1);
	m_pSR4000Conf = cvCreateImage( cvSize(frameWidth,frameHeight), IPL_DEPTH_8U, 1);
	m_pSR4000Ampl = cvCreateImage( cvSize(frameWidth,frameHeight), IPL_DEPTH_8U, 1);
	if( m_pSR4000Dist == NULL || m_pSR4000Conf == NULL || m_pSR4000Ampl == NULL )
	{
		fprintf( stderr, "ERROR: SwissRanger %d : cvCreateImage failed.\n", numCam);
		return -1;
	}

	m_fDmax = Distance[ m_pFrequency ];

	// initialization is ok ...
	sprintf( name, "SwissRanger-%d", numCam);
	fprintf( stderr, "SUCCESS: %s detected.\n", name);
	setActive(true);

	return 0;
}

//-------------------------------------------------------------------

void CameraSwissRanger::close(void) 
{
	SR_Close( m_pSwissRangerCapture );

	delete[] m_pXYZCoords;
	m_pXYZCoords = NULL;
	
	if( m_pSR4000Dist != NULL )
		cvReleaseImage(&m_pSR4000Dist);
	if( m_pSR4000Conf != NULL )
		cvReleaseImage(&m_pSR4000Conf);
	if( m_pSR4000Ampl != NULL )
		cvReleaseImage(&m_pSR4000Ampl);
	m_pSR4000Dist = NULL;
	m_pSR4000Conf = NULL;
	m_pSR4000Ampl = NULL;
	
	lastFrame = NULL;
	imgCnt = 0;
	
	Camera::close();
}

//-------------------------------------------------------------------

void* CameraSwissRanger::getRawDataPtr(void)
{
	int iResult = 0;

	iResult = SR_Acquire(m_pSwissRangerCapture);
	if( iResult < 0 )
	{
#ifdef _DEBUG
		fprintf( stderr, "%s : SR_Acquire failed in getRawDataPtr.\n", name);
#endif
	 	return NULL;
	}

	imgCnt = SR_GetImageList( m_pSwissRangerCapture, &m_pImgEntryArray);
	if( imgCnt < 0 )
	{
#ifdef _DEBUG
		 fprintf( stderr, "%s : SR_GetImageList failed in getRawDataPtr.\n", name);
#endif
		return NULL;
	}
	// ! image count depends on acquire mode !
	// if AM_CONF_MAP flag is not set in acquire mode,
	// confidence image is not available.
	if( outputImageType > imgCnt-1 )
		outputImageType = IMGTYPE_DISTANCE;

   	return (void*) m_pImgEntryArray[outputImageType].data;
}

//-------------------------------------------------------------------

IplImage* CameraSwissRanger::get1Frame(void)
{
	int iResult = 0;
	void* m_pBuffer = NULL; // Buffer to save data from SwissRanger.
	float fValue;

	getRawDataPtr();

	// ! image count depends on acquire mode !
	// if AM_CONF_MAP flag not set in acquire mode,
	// confidence image will not be updated.
	for( int iImg = 0; iImg < imgCnt; iImg++)
	{
		switch( m_pImgEntryArray[iImg].imgType )
		{
			case _ImgEntry::IT_DISTANCE :
			{
				// distance image
				m_pBuffer = m_pImgEntryArray[iImg].data;
	
				iResult = SR_CoordTrfFlt( m_pSwissRangerCapture, &m_pXYZCoords[0], &m_pXYZCoords[1], &m_pXYZCoords[2], sizeof(float[3]) , sizeof(float[3]) , sizeof(float[3]));
				//iResult = SR_CoordTrfFlt( m_pSwissRangerCapture, m_pXCoords, m_pYCoords, m_pZCoords, sizeof(float) , sizeof(float) , sizeof(float) );
				unsigned char* pImageData = (uchar*)m_pSR4000Dist->imageData;

				for( uint i = 0 ; i < frameWidth * frameHeight ; i++ )
				{
					//fValue = (m_pZCoords[ i ] * ( (float) ( 1 << 8 ) / Distance[ m_pFrequency ] ) );
					fValue = (m_pXYZCoords[i*3 + 2] * ( (float) ( 1 << 8 ) / Distance[ m_pFrequency ] ) );
					pImageData[ i ] = (uchar) CLAMP( fValue, 0, 255 );
				}
			}
			break;
			
			case _ImgEntry::IT_AMPLITUDE :
			{
				// amplitude image
				unsigned short int* pAmplitudeBuffer = (unsigned short int*) m_pImgEntryArray[iImg].data;
				uchar* pAmplitudeData = (uchar*)m_pSR4000Ampl->imageData;

				for( uint i = 0 ; i < frameWidth * frameHeight ; i++  )
				{
					pAmplitudeData[ i ] = (uchar) (pAmplitudeBuffer[ i ] >> 8 );
				}
			}
			break;
			
			case _ImgEntry::IT_CONF_MAP :
			{
				// confidence image
				m_pBuffer = (short int*) m_pImgEntryArray[iImg].data;
				unsigned short int* pConfidentBuffer = (unsigned short int*) m_pImgEntryArray[iImg].data;
				uchar* pConfidentData = (uchar*) m_pSR4000Conf->imageData;

				for( uint i = 0 ; i < frameWidth*frameHeight ; i++  )
				{
					pConfidentData[ i ] = (uchar) (pConfidentBuffer[ i ] >> 8 );
				}
			}
			break;
		}	
	}
	
	// output image depends on outputImageType
	if( outputImageType == IMGTYPE_DISTANCE )
		lastFrame = m_pSR4000Dist;
	else if( outputImageType == IMGTYPE_CONFIDENCE )
		lastFrame = m_pSR4000Conf;
	else // default is AMPLITUDE
		lastFrame = m_pSR4000Ampl;
		
	if( isUndistortModeOn() )
		undistortFrame(lastFrame);
		
	return lastFrame;
}

//-------------------------------------------------------------------

int CameraSwissRanger::getParameter(int control)
{
	int iResult = 0;

	switch( control )
	{
		case PARAM_ACQUIREMODE :
			iResult = SR_GetMode(m_pSwissRangerCapture);
		break;
		
		case PARAM_TIMEOUT :
			// not readable
		break;
		
		case PARAM_INTEGRATIONTIME :
			iResult = SR_GetIntegrationTime(m_pSwissRangerCapture);
		break;
		
		case PARAM_AMPLITUDETHRESHOLD :
			iResult = SR_GetAmplitudeThreshold(m_pSwissRangerCapture);
		break;
		
		case PARAM_MODULATIONFREQUENCY :
			iResult = SR_GetModulationFrequency(m_pSwissRangerCapture);
		break;
		
		case PARAM_DISTANCEOFFSET :
			iResult = SR_GetDistanceOffset(m_pSwissRangerCapture);
		break;
		
		case PARAM_AUTOEXPOSURE_ONOFF :
		case PARAM_AUTOEXPOSURE_MININTTIME :
		case PARAM_AUTOEXPOSURE_MAXINTTIME :
		case PARAM_AUTOEXPOSURE_PERCENTOVERPOS :
		case PARAM_AUTOEXPOSURE_DESIREDPOS :
			// not readable
			iResult = -1;
		break;
		
		case PARAM_OUTPUTIMGTYPE :
			iResult = outputImageType;
		break;
	}

	return iResult;
}

//-------------------------------------------------------------------

int CameraSwissRanger::setParameter(int control, int value)
{
	int iResult = 0;

	switch( control )
	{
		case PARAM_ACQUIREMODE :
			iResult = SR_SetMode( m_pSwissRangerCapture, value);
		break;
		
		case PARAM_TIMEOUT :
			SR_SetTimeout( m_pSwissRangerCapture, value);
		break;
		
		case PARAM_INTEGRATIONTIME :
			if( value >= 0 && value <= 255 )
				iResult = SR_SetIntegrationTime( m_pSwissRangerCapture, (unsigned char) value);
			else
			{
				fprintf( stderr, "Value out of range. Integration time is a value from 0 to 255.\n");
				iResult = -1;
			}
		break;

		case PARAM_AMPLITUDETHRESHOLD :
			iResult = SR_SetAmplitudeThreshold( m_pSwissRangerCapture, (unsigned short) value);
		break;

		case PARAM_MODULATIONFREQUENCY :
			iResult = SR_SetModulationFrequency( m_pSwissRangerCapture, (ModulationFrq) value);
		break;

		case PARAM_DISTANCEOFFSET :
			iResult = SR_SetDistanceOffset( m_pSwissRangerCapture, (unsigned short) value);
		break;

		case PARAM_AUTOEXPOSURE_ONOFF :
			// <=0 value to disable, >0 value to enable
			if( value > 0 )
				iResult = SR_SetAutoExposure( m_pSwissRangerCapture, autoExposure.minIntTime, autoExposure.maxIntTime, autoExposure.percentOverPos, autoExposure.desiredPos);
			else
			{
				// MesaSR lib doc : "if minIntTime=0xff the AutoExposure is turned off."
				iResult = SR_SetAutoExposure( m_pSwissRangerCapture, 0xff, 0, 0, 0);
			}
		break;

		case PARAM_AUTOEXPOSURE_MININTTIME :
			if( value >= 0 && value <= 255 )
				autoExposure.minIntTime = (unsigned char) value;
			else
			{
				fprintf( stderr, "Value out of range. Auto exposure minIntTime is a value from 0 to 255.\n");
				iResult = -1;
			}
		break;
		
		case PARAM_AUTOEXPOSURE_MAXINTTIME :
			if( value >= 0 && value <= 255 )
				autoExposure.maxIntTime = (unsigned char) value;
			else
			{
				fprintf( stderr, "Value out of range. Auto exposure maxIntTime is a value from 0 to 255.\n");
				iResult = -1;
			}
		break;
		
		case PARAM_AUTOEXPOSURE_PERCENTOVERPOS :
			if( value >= 0 && value <= 255 )
				autoExposure.percentOverPos = (unsigned char) value;
			else
			{
				fprintf( stderr, "Value out of range. Auto exposure percentOverPos is a value from 0 to 255.\n");
				iResult = -1;
			}
		break;
		
		case PARAM_AUTOEXPOSURE_DESIREDPOS :
			if( value >= 0 && value <= 255 )
				autoExposure.desiredPos = (unsigned char) value;
			else
			{
				fprintf( stderr, "Value out of range. Auto exposure desiredPos is a value from 0 to 255.\n");
				iResult = -1;
			}
		break;

		case PARAM_OUTPUTIMGTYPE :
			if( value >= IMGTYPE_DISTANCE && value <= IMGTYPE_CONFIDENCE )
				outputImageType = value;
			else
			{
				fprintf( stderr, "Value out of range. Image type is a value from 0 to 2.\n");
				iResult = -1;
			}
		break;
	}

	// MesaSR lib documentation :
	// "Most of the functions return an int value. If not explicitly documented,
	// a zero or positive return value means 'successful' and any negative value
	// indicates an error."
	if( iResult < 0 )
    {
        fprintf( stderr, "CameraSwissRanger::setParameter( %d, %d) failed, with return value of %d !!\n", control, value, iResult);
        return -1;
    }

	return 0;
}

//-------------------------------------------------------------------

void CameraSwissRanger::listParameters(void)
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
			case PARAM_ACQUIREMODE :
				// binary flags
				minVal = -1;
				maxVal = -1;
				step = -1;
			break;

			case PARAM_TIMEOUT :
				minVal = 0;
				maxVal = 65535;
				step = 1;
			break;

			case PARAM_INTEGRATIONTIME :
				minVal = 0;
				maxVal = 255;
				step = 1;
			break;

			case PARAM_AMPLITUDETHRESHOLD :
				minVal = 0;
				maxVal = 65535;
				step = 1;
			break;

			case PARAM_MODULATIONFREQUENCY :
				// enum
				minVal = -1;
				maxVal = -1;
				step = -1;
			break;

			case PARAM_DISTANCEOFFSET :
				minVal = 0;
				maxVal = 65535;
				step = 1;
			break;

			case PARAM_AUTOEXPOSURE_ONOFF :
				minVal = 0;
				maxVal = 1;
				step = 1;
			break;

			case PARAM_AUTOEXPOSURE_MININTTIME :
				minVal = 0;
				maxVal = 255;
				step = 1;
			break;
		
			case PARAM_AUTOEXPOSURE_MAXINTTIME :
				minVal = 0;
				maxVal = 255;
				step = 1;
			break;

			case PARAM_AUTOEXPOSURE_PERCENTOVERPOS :
				minVal = 0;
				maxVal = 255;
				step = 1;
			break;

			case PARAM_AUTOEXPOSURE_DESIREDPOS :
				minVal = 0;
				maxVal = 255;
				step = 1;
			break;

			case PARAM_OUTPUTIMGTYPE :
				minVal = IMGTYPE_DISTANCE;
				maxVal = IMGTYPE_CONFIDENCE;
				step = 1;
			break;
		}
		
		getCameraParameterName( iPar, parameterName, bufSize);
		printf( "%35s %8d %8ld %8ld %8d\n", parameterName, getParameter(iPar), minVal, maxVal, step); 
	}
	
	printf("\n");
}

//-------------------------------------------------------------------

void CameraSwissRanger::getCameraParameterName( int parId, char* buffer, int bufSize)
{
	switch( parId )
	{
		case PARAM_ACQUIREMODE :
			snprintf( buffer, bufSize, "%s", "PARAM_ACQUIREMODE");
		break;

		case PARAM_TIMEOUT :
			snprintf( buffer, bufSize, "%s", "PARAM_TIMEOUT");
		break;

		case PARAM_INTEGRATIONTIME :
			snprintf( buffer, bufSize, "%s", "PARAM_INTEGRATIONTIME");
		break;

		case PARAM_AMPLITUDETHRESHOLD :
			snprintf( buffer, bufSize, "%s", "PARAM_AMPLITUDETHRESHOLD");
		break;

		case PARAM_MODULATIONFREQUENCY :
			snprintf( buffer, bufSize, "%s", "PARAM_MODULATIONFREQUENCY");
		break;

		case PARAM_DISTANCEOFFSET :
			snprintf( buffer, bufSize, "%s", "PARAM_DISTANCEOFFSET");
		break;

		case PARAM_AUTOEXPOSURE_ONOFF :
			snprintf( buffer, bufSize, "%s", "PARAM_AUTOEXPOSURE_ONOFF");
		break;

		case PARAM_AUTOEXPOSURE_MININTTIME :
			snprintf( buffer, bufSize, "%s", "PARAM_AUTOEXPOSURE_MININTTIME");
		break;

		case PARAM_AUTOEXPOSURE_MAXINTTIME :
			snprintf( buffer, bufSize, "%s", "PARAM_AUTOEXPOSURE_MAXINTTIME");
		break;

		case PARAM_AUTOEXPOSURE_PERCENTOVERPOS :
			snprintf( buffer, bufSize, "%s", "PARAM_AUTOEXPOSURE_PERCENTOVERPOS");
		break;

		case PARAM_AUTOEXPOSURE_DESIREDPOS :
			snprintf( buffer, bufSize, "%s", "PARAM_AUTOEXPOSURE_DESIREDPOS");
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

void CameraSwissRanger::recordFrame(void)
{
	static int recFrameCnt = 0;
	static char bufferCnt[32]; 

	recFrameCnt++;

	// build amplitude image file name
	std::string fileName(getName());
	snprintf( bufferCnt, sizeof(bufferCnt), "-amplitude-%06d", recFrameCnt);
	fileName.append(bufferCnt);
	fileName.append(".pgm");
	
	// write amplitude image to file
	cvSaveImage( fileName.c_str(), getAmplitude());

	// build confidence image file name
	fileName.assign(getName());
	snprintf( bufferCnt, sizeof(bufferCnt), "-confidence-%06d", recFrameCnt);
	fileName.append(bufferCnt);
	fileName.append(".pgm");
	
	// write confidence image to file
	cvSaveImage( fileName.c_str(), getConfidence());

	// build distance image file name
	fileName.assign(getName());
	snprintf( bufferCnt, sizeof(bufferCnt), "-distance-%06d", recFrameCnt);
	fileName.append(bufferCnt);
	fileName.append(".pgm");
	
	// write amplitude image to file
	cvSaveImage( fileName.c_str(), getDistance());
}


//-------------------------------------------------------------------

} // namespace apicamera
