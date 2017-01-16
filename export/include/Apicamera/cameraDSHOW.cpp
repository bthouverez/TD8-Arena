#ifdef D_API_WIN32	// windows only


#include "cameraDSHOW.h"
#include "dshow_utils.h"

#include <stdio.h>
#include <string>


// the class factories

extern "C" DLL_EXPORT apicamera::Camera* create(void)
{
    return new apicamera::CameraDSHOW;
}

extern "C" DLL_EXPORT void destroy(apicamera::Camera* p) 
{
    delete p;
}


namespace apicamera
{

//-------------------------------------------------------------------

CameraDSHOW::CameraDSHOW()
{
#ifdef _DEBUG
	//printf( "ici CameraDSHOW::CameraDSHOW()\n");
#endif
	
	// must initialize COM before using DSHOW
	openCOM();

	pSrcFilter = NULL;
	g_pGraph = NULL;
	g_pCapture = NULL;
	g_pMC = NULL;
	pGrabberF = NULL;
	pGrabber = NULL;
	pCaptureConfig = NULL;
	pProcAmp = NULL;
	pControlCam = NULL;
	pKsPropertySet = NULL;
}

//-------------------------------------------------------------------

CameraDSHOW::~CameraDSHOW()
{
#ifdef _DEBUG
	//printf( "ici CameraDSHOW::~CameraDSHOW()\n");
#endif
	close();
	closeCOM();
}

//-------------------------------------------------------------------

int CameraDSHOW::open( int numCam, unsigned int width, unsigned int height, unsigned int fRate, unsigned int format)
{
	// détecte caméra 
	if ( ! SUCCEEDED( FindCaptureDevice( &pSrcFilter, numCam)) )
	{
        fprintf( stderr, "CameraDSHOW::open : camera %d not detected \n", numCam);
		return -1;
	}

	// construit un graphe DSHOW
	if( ! SUCCEEDED( buildGraph()) )
	{
        fprintf( stderr, "CameraDSHOW::open : DSHOW graph building failed for camera %d\n", numCam);
		return -1;
	}

	// fixe la resolution et le framerate
	if( setResolutionAndFramerate( width, height, fRate) != 0 )
	{
        fprintf( stderr, "CameraDSHOW::open : setting resolution and framerate failed for camera %d\n", numCam);
		return -1;
	}

    frameWidth = width;
    frameHeight = height;
    framerate = fRate;
    frameFormat = format;

    // memory allocation for BGR image !!
    lastFrame = cvCreateImage( cvSize( frameWidth, frameHeight), IPL_DEPTH_8U, 3);
    if( lastFrame == NULL )
    {
        fprintf( stderr, "CameraDSHOW::open : camera %d : cvCreateImage failed.\n", numCam);
		return -1;
	}

    // si tout c'est bien passe ...
	sprintf( name, "DSHOW-%d", numCam);
	fprintf( stderr, "CameraDSHOW::open : %s detected \n", name);
	setActive(true);

	return 0;
}

//-------------------------------------------------------------------

int CameraDSHOW::open( int numCam, OpenParameters *param)
{
	return open( numCam, param->width, param->height, param->fRate, 0);		
}

//-------------------------------------------------------------------

void CameraDSHOW::close(void)
{
	// arrête le graphe
	if( g_pMC != NULL )
		g_pMC->Stop();

	// libere la memoire
	if( pKsPropertySet != NULL )
		pKsPropertySet->Release();
	if( pProcAmp != NULL )
		pProcAmp->Release();
	if( pControlCam != NULL )
		pControlCam->Release();
	if( pCaptureConfig != NULL )
		pCaptureConfig->Release();
	if( pGrabber != NULL )
		pGrabber->Release();
	if( pGrabberF != NULL )
		pGrabberF->Release();
	if( g_pMC != NULL )
		g_pMC->Release();
	if( g_pCapture != NULL )
		g_pCapture->Release();
	if( g_pGraph != NULL )
		g_pGraph->Release();
	if( pSrcFilter != NULL )
		pSrcFilter->Release();
		
	pSrcFilter = NULL;
	g_pGraph = NULL;
	g_pCapture = NULL;
	g_pMC = NULL;
	pGrabberF = NULL;
	pGrabber = NULL;
	pCaptureConfig = NULL;
	pProcAmp = NULL;
	pControlCam = NULL;
	pKsPropertySet = NULL;
		
	Camera::close();
}

//-------------------------------------------------------------------

IplImage* CameraDSHOW::get1Frame(void)
{
/*
	// trouver la taille du buffer du sample grabber
	// GetCurrentBuffer( &grabberBufferSize, NULL) ne repond
	// qu'apres N appels ...
	long grabberBufferSize = 0;
	unsigned long nbAppel = 0;
	while ( FAILED(hr) || grabberBufferSize == 0 )
	{
		hr = pGrabber->GetCurrentBuffer( &grabberBufferSize, NULL);
		nbAppel++;
	}
	fprintf( stderr, "grabberBufferSize = %ld apres %lu appels !\n", grabberBufferSize, nbAppel);
*/

	long bufferSize = lastFrame->imageSize;

	if( FAILED( pGrabber->GetCurrentBuffer( &bufferSize, (long*) lastFrame->imageData)) )
		fprintf( stderr, "%s : CameraDSHOW::get1Frame failed.\n", name);
	else
		cvFlip( lastFrame, NULL, 0);

	if( isUndistortModeOn() )
		undistortFrame(lastFrame);
	
	return  lastFrame;
}

//-------------------------------------------------------------------

int CameraDSHOW::getParameter(int control)
{
	HRESULT hr = E_NOTIMPL;
	long value, flag;

	// parametres génériques signal video
	switch( control )
	{
		case PARAM_BRIGHTNESS :
			hr = pProcAmp->Get( VideoProcAmp_Brightness, &value, &flag);
			break;
		case PARAM_CONTRAST :
			hr = pProcAmp->Get( VideoProcAmp_Contrast, &value, &flag);
			break;
		case PARAM_HUE :
			hr = pProcAmp->Get( VideoProcAmp_Hue, &value, &flag);
			break;
		case PARAM_SATURATION :
			hr = pProcAmp->Get( VideoProcAmp_Saturation, &value, &flag);
			break;
		case PARAM_SHARPNESS :
			hr = pProcAmp->Get( VideoProcAmp_Sharpness, &value, &flag);
			break;
		case PARAM_GAMMA :
			hr = pProcAmp->Get( VideoProcAmp_Gamma, &value, &flag);
			break;
		case PARAM_COLORENABLE :
			hr = pProcAmp->Get( VideoProcAmp_ColorEnable, &value, &flag);
			break;
		case PARAM_WHITEBALANCE :
			hr = pProcAmp->Get( VideoProcAmp_WhiteBalance, &value, &flag);
			break;
		case PARAM_BACKLIGHTCOMPENSATION :
			hr = pProcAmp->Get( VideoProcAmp_BacklightCompensation, &value, &flag);
			break;
		case PARAM_GAIN :
			hr = pProcAmp->Get( VideoProcAmp_Gain, &value, &flag);
			break;
	}

	// parametres specifiques cameras, non gérés par toutes les cameras
	if ( pControlCam != NULL )
	{
		switch( control )
		{
			case PARAM_PAN :
				hr = pControlCam->Get( CameraControl_Pan, &value, &flag);
				break;
			case PARAM_TILT :
				hr = pControlCam->Get( CameraControl_Tilt, &value, &flag);
				break;
			case PARAM_ROLL :
				hr = pControlCam->Get( CameraControl_Roll, &value, &flag);
				break;
			case PARAM_ZOOM :
				hr = pControlCam->Get( CameraControl_Zoom, &value, &flag);
				break;
			case PARAM_EXPOSURE :
				hr = pControlCam->Get( CameraControl_Exposure, &value, &flag);
				break;
			case PARAM_IRIS :
				hr = pControlCam->Get( CameraControl_Iris, &value, &flag);
				break;
			case PARAM_FOCUS :
				hr = pControlCam->Get( CameraControl_Focus, &value, &flag);
				break;
		}
	}

	// special parameters accessed by KsPropertySet Dshow interface
	if ( pKsPropertySet != NULL )
	{
		switch( control )
		{
			case PARAM_LOGITECH_LP1_EXPOSURE_TIME :
				KSPROPERTY_LP1_EXPOSURE_TIME_S expTime;
				DWORD dwReturned = 0;
				
				hr = pKsPropertySet->Get( PROPSETID_LOGITECH_PUBLIC1, KSPROPERTY_LP1_EXPOSURE_TIME, NULL, 0, &expTime, sizeof(expTime), &dwReturned);
				value = expTime.ulExposureTime;
				break;
		}
	}

	if( FAILED(hr) )
    {
        fprintf( stderr, "CameraDSHOW::getParameter( %d) failed !!\n", control);
        return -1;
    }

	return value;
}

//-------------------------------------------------------------------

int CameraDSHOW::setParameter(int control, int value)
{
	HRESULT hr = E_NOTIMPL;

	// parametres génériques signal video
	switch( control )
	{
		case PARAM_BRIGHTNESS :
			hr = pProcAmp->Set( VideoProcAmp_Brightness, value, VideoProcAmp_Flags_Manual);
			break;
		case PARAM_CONTRAST :
			hr = pProcAmp->Set( VideoProcAmp_Contrast, value, VideoProcAmp_Flags_Manual);
			break;
		case PARAM_HUE :
			hr = pProcAmp->Set( VideoProcAmp_Hue, value, VideoProcAmp_Flags_Manual);
			break;
		case PARAM_SATURATION :
			hr = pProcAmp->Set( VideoProcAmp_Saturation, value, VideoProcAmp_Flags_Manual);
			break;
		case PARAM_SHARPNESS :
			hr = pProcAmp->Set( VideoProcAmp_Sharpness, value, VideoProcAmp_Flags_Manual);
			break;
		case PARAM_GAMMA :
			hr = pProcAmp->Set( VideoProcAmp_Gamma, value, VideoProcAmp_Flags_Manual);
			break;
		case PARAM_COLORENABLE :
			hr = pProcAmp->Set( VideoProcAmp_ColorEnable, value, VideoProcAmp_Flags_Manual);
			break;
		case PARAM_WHITEBALANCE :
			hr = pProcAmp->Set( VideoProcAmp_WhiteBalance, value, VideoProcAmp_Flags_Manual);
			break;
		case PARAM_BACKLIGHTCOMPENSATION :
			hr = pProcAmp->Set( VideoProcAmp_BacklightCompensation, value, VideoProcAmp_Flags_Manual);
			break;
		case PARAM_GAIN :
			hr = pProcAmp->Set( VideoProcAmp_Gain, value, VideoProcAmp_Flags_Manual);
			break;
	}

	// parametres specifiques cameras, non gérés par toutes les cameras
	if ( pControlCam != NULL )
	{
		switch( control )
		{
			case PARAM_PAN :
				hr = pControlCam->Set( CameraControl_Pan, value, CameraControl_Flags_Manual);
				break;
			case PARAM_TILT :
				hr = pControlCam->Set( CameraControl_Tilt, value, CameraControl_Flags_Manual);
				break;
			case PARAM_ROLL :
				hr = pControlCam->Set( CameraControl_Roll, value, CameraControl_Flags_Manual);
				break;
			case PARAM_ZOOM :
				hr = pControlCam->Set( CameraControl_Zoom, value, CameraControl_Flags_Manual);
				break;
			case PARAM_EXPOSURE :
				hr = pControlCam->Set( CameraControl_Exposure, value, CameraControl_Flags_Manual);
				break;
			case PARAM_IRIS :
				hr = pControlCam->Set( CameraControl_Iris, value, CameraControl_Flags_Manual);
				break;
			case PARAM_FOCUS :
				hr = pControlCam->Set( CameraControl_Focus, value, CameraControl_Flags_Manual);
				break;
		}
	}

	// special parameters accessed by KsPropertySet Dshow interface
	if ( pKsPropertySet != NULL )
	{
		switch( control )
		{
			case PARAM_LOGITECH_LP1_EXPOSURE_TIME :
				KSPROPERTY_LP1_EXPOSURE_TIME_S expTime;
				DWORD dwReturned = 0;
				/*
				hr = pKsPropertySet->QuerySupported(PROPSETID_LOGITECH_PUBLIC1, KSPROPERTY_LP1_EXPOSURE_TIME, &dwReturned);

				hr = pKsPropertySet->Get( PROPSETID_LOGITECH_PUBLIC1, KSPROPERTY_LP1_EXPOSURE_TIME, NULL, 0, &expTime, sizeof(expTime), &dwReturned);
				*/
				expTime.ulExposureTime = value;
				expTime.Header.ulFlags = KSPROPERTY_CAMERACONTROL_FLAGS_MANUAL;
				hr = pKsPropertySet->Set( PROPSETID_LOGITECH_PUBLIC1, KSPROPERTY_LP1_EXPOSURE_TIME, NULL, 0, &expTime, sizeof(expTime));
				break;
		}
	}


	if( FAILED(hr) )
    {
        fprintf( stderr, "CameraDSHOW::setParameter( %d, %d) failed !!\n", control, value);
        return -1;
    }

	return 0;
}

//-------------------------------------------------------------------

void CameraDSHOW::listParameters(void)
{
	// appelle l'interface de gestion du flux (stream) du graphe de capture 
	int iCount, iSize, t=0;
	std::string mediaSubTypeString;
	mediaSubTypeString.reserve(64);
	
	// recherche le nombre de formats disponibles pour le flux
	pCaptureConfig->GetNumberOfCapabilities(&iCount, &iSize);
	printf( "%d picture format(s) available :\n", iCount);
	
	// etude des formats disponibles
	for( int i = 0; i < iCount; i++)
	{
		AM_MEDIA_TYPE* sCapType;
		VIDEO_STREAM_CONFIG_CAPS sCapCaps;

		// récupération des informations concernant le type de flux i
		pCaptureConfig->GetStreamCaps(i, &sCapType, (BYTE*)&sCapCaps);


		// explicite le media sub type
		convertMediaSubTypeGUIDToString( sCapType->subtype, mediaSubTypeString);

		// affiche les infos sur le format
		VIDEOINFOHEADER *pVIH = (VIDEOINFOHEADER*)sCapType->pbFormat;
		BITMAPINFOHEADER Header=pVIH->bmiHeader;
		printf( " Media sub type %s, resolution %d x %d\n", mediaSubTypeString.c_str(),
			Header.biWidth, Header.biHeight);

		// free memory
		MyDeleteMediaType(sCapType);
	}

	// print camera parameters range
	printf( "Parameters :\n");

	long vMin, vMax, vStep, vDefault, vCapsFlags;
	std::string buffer;
	buffer.reserve(64);
	
	// affichage des parametres génériques du signal video
	printf( " - generic video signal parameters :\n");
	for( int i = 0; i < PARAM_GEN_END; i++)
	{
		pProcAmp->GetRange( convertParameterToDshowValue(i), &vMin, &vMax, &vStep, &vDefault, &vCapsFlags);
		printf( "   %s : min=%ld  max=%ld  step=%ld  default=%ld  capsflags=%ld\n", 
			convertParameterToString( i, buffer), vMin, vMax, vStep, vDefault, vCapsFlags);
	}

	// affichage des parametres spécifiques de caméra
	printf( " - specific camera parameters :\n");
	if ( pControlCam != NULL )	
	{
		for( int i = PARAM_GEN_END+1; i < PARAM_SPE_END; i++)
		{
			pControlCam->GetRange( convertParameterToDshowValue(i), &vMin, &vMax, &vStep, 
				&vDefault, &vCapsFlags);
			printf( "   %s : min=%ld max=%ld  step=%ld  default=%ld  capsflags=%ld\n", 
				convertParameterToString( i, buffer), vMin, vMax, vStep, vDefault,
				vCapsFlags);
		}
	}
	else
		printf( "   not supported by this camera.\n");
}

//-------------------------------------------------------------------

int CameraDSHOW::setResolutionAndFramerate( unsigned int width, unsigned int height, unsigned int fRate)
{
	HRESULT hr;
	int iCount, iSize, result = -1;
	AM_MEDIA_TYPE* bestFormat = NULL;
	VIDEOINFOHEADER *pVIH;
	BITMAPINFOHEADER Header;

	
	// recherche du nombre de formats disponibles pour le flux
	pCaptureConfig->GetNumberOfCapabilities(&iCount, &iSize);
	
	// parcours des formats disponibles qui sont codés en RGB24 ou IYUV
	for( int i=0; i < iCount; i++)
	{
		AM_MEDIA_TYPE* sCapType;
		VIDEO_STREAM_CONFIG_CAPS sCapCaps;

		//récupération des informations concernant le type de flux i
		pCaptureConfig->GetStreamCaps(i, &sCapType, (BYTE*)&sCapCaps);
		
		// on recherche la resolution demandee en format RGB24, ou à défault, en IYUV
		if ( sCapType->subtype == MEDIASUBTYPE_RGB24 || 
			( bestFormat == NULL && sCapType->subtype == MEDIASUBTYPE_IYUV ) )
		{
			pVIH = (VIDEOINFOHEADER*)sCapType->pbFormat;
			Header = pVIH->bmiHeader;

			if( (Header.biWidth == width) && (Header.biHeight == height) )
			{
				MyDeleteMediaType(bestFormat);
				pCaptureConfig->GetStreamCaps(i, &bestFormat, (BYTE*)&sCapCaps);
			}
		}

		// free memory
		MyDeleteMediaType(sCapType);
	}

	// si on a trouve un bon format ...
	if( bestFormat != NULL )
	{
		pVIH = (VIDEOINFOHEADER*) bestFormat->pbFormat;
		Header = pVIH->bmiHeader;

		// fixe le framerate
		// tps d'affichage d'un frame en 100 ns
		pVIH->AvgTimePerFrame = 10000000 / fRate;

		// arrête le graphe
		g_pMC->Stop();        

		// fixe le format
		hr = pCaptureConfig->SetFormat( bestFormat );
				
		if( FAILED(hr) )
			fprintf( stderr, "CameraDSHOW::setResolutionAndFramerate : failed.\n");
		else
		{
			fprintf( stderr, "CameraDSHOW::setResolutionAndFramerate : succeeded.\n");
			result = 0;
		}

		// re-demarre le graphe
		hr = g_pMC->Run();        
		if (FAILED(hr))
		{
			fprintf( stderr, "CameraDSHOW::setResolutionAndFramerate : Couldn't run the graph!\n");
			return hr;
		}
	}
				
	// free memory
	MyDeleteMediaType(bestFormat);

	return result;		// 0 = OK, -1 = ERROR
}

//-------------------------------------------------------------------

const char* CameraDSHOW::convertParameterToString( int param, std::string& buffer)
{
	switch( param )
	{
		case CameraDSHOW::PARAM_BRIGHTNESS :
			buffer.assign( "brightness" );
			break;
		case CameraDSHOW::PARAM_CONTRAST :
			buffer.assign( "contrast" );
			break;
		case CameraDSHOW::PARAM_HUE :
			buffer.assign( "hue" );
			break;
		case CameraDSHOW::PARAM_SATURATION :
			buffer.assign( "saturation" );
			break;
		case CameraDSHOW::PARAM_SHARPNESS :
			buffer.assign( "sharpness" );
			break;
		case CameraDSHOW::PARAM_GAMMA :
			buffer.assign( "gamma" );
			break;
		case CameraDSHOW::PARAM_COLORENABLE :
			buffer.assign( "colorEnable" );
			break;
		case CameraDSHOW::PARAM_WHITEBALANCE :
			buffer.assign( "whiteBalance" );
			break;
		case CameraDSHOW::PARAM_BACKLIGHTCOMPENSATION :
			buffer.assign( "backlight-compensation" );
			break;
		case CameraDSHOW::PARAM_GAIN :
			buffer.assign( "gain" );
			break;
	
		// parametres specifiques cameras, non gérés par toutes les cameras
		case CameraDSHOW::PARAM_PAN :
			buffer.assign( "pan" );
			break;
		case CameraDSHOW::PARAM_TILT :
			buffer.assign( "tilt" );
			break;
		case CameraDSHOW::PARAM_ROLL :
			buffer.assign( "roll" );
			break;
		case CameraDSHOW::PARAM_ZOOM :
			buffer.assign( "zoom" );
			break;
		case CameraDSHOW::PARAM_EXPOSURE :
			buffer.assign( "exposure" );
			break;
		case CameraDSHOW::PARAM_IRIS :
			buffer.assign( "iris" );
			break;
		case CameraDSHOW::PARAM_FOCUS :
			buffer.assign( "focus" );
			break;
		default :
			buffer.assign( "unknown" );
	}

	return buffer.c_str();
}

//-------------------------------------------------------------------

long CameraDSHOW::convertParameterToDshowValue( int param)
{
	switch( param )
	{
		case CameraDSHOW::PARAM_BRIGHTNESS :
			return VideoProcAmp_Brightness;
			break;
		case CameraDSHOW::PARAM_CONTRAST :
			return VideoProcAmp_Contrast;
			break;
		case CameraDSHOW::PARAM_HUE :
			return VideoProcAmp_Hue;
			break;
		case CameraDSHOW::PARAM_SATURATION :
			return VideoProcAmp_Saturation;
			break;
		case CameraDSHOW::PARAM_SHARPNESS :
			return VideoProcAmp_Sharpness;
			break;
		case CameraDSHOW::PARAM_GAMMA :
			return VideoProcAmp_Gamma;
			break;
		case CameraDSHOW::PARAM_COLORENABLE :
			return VideoProcAmp_ColorEnable;
			break;
		case CameraDSHOW::PARAM_WHITEBALANCE :
			return VideoProcAmp_WhiteBalance;
			break;
		case CameraDSHOW::PARAM_BACKLIGHTCOMPENSATION :
			return VideoProcAmp_BacklightCompensation;
			break;
		case CameraDSHOW::PARAM_GAIN :
			return VideoProcAmp_Gain;
			break;
	
		// parametres specifiques cameras, non gérés par toutes les cameras
		case CameraDSHOW::PARAM_PAN :
			return CameraControl_Pan;
			break;
		case CameraDSHOW::PARAM_TILT :
			return CameraControl_Tilt;
			break;
		case CameraDSHOW::PARAM_ROLL :
			return CameraControl_Roll;
			break;
		case CameraDSHOW::PARAM_ZOOM :
			return CameraControl_Zoom;
			break;
		case CameraDSHOW::PARAM_EXPOSURE :
			return CameraControl_Exposure;
			break;
		case CameraDSHOW::PARAM_IRIS :
			return CameraControl_Iris;
			break;
		case CameraDSHOW::PARAM_FOCUS :
			return CameraControl_Focus;
			break;
		default :
			return -1;
	}
}

//-------------------------------------------------------------------

/**********************************************/
/*       DSHOW  :(                     */
/**********************************************/

unsigned int CameraDSHOW::COMinitialized = 0;

//-------------------------------------------------------------------

int CameraDSHOW::openCOM(void)
{
    HRESULT hr = S_OK;

#ifdef _DEBUG
	printf( "%s : COMinitialized=%u\n", __FUNCTION__, COMinitialized);
#endif

	if( COMinitialized == 0 )
	{
		// Initialize the COM library.
		hr = CoInitialize(NULL);
		if (FAILED(hr))
		{
			printf("ERROR - Could not initialize COM library");
			return -1;
		}

	#ifdef _DEBUG
		printf( "%s : COM really initialized\n", __FUNCTION__);
	#endif
	}

	COMinitialized++;

	return 0;
}

//-------------------------------------------------------------------

void CameraDSHOW::closeCOM(void)
{
#ifdef _DEBUG
	printf( "%s : COMinitialized=%u\n", __FUNCTION__, COMinitialized);
#endif

	// fin d'utilisation de COM
	if( COMinitialized == 1 )
	{
		CoUninitialize();

	#ifdef _DEBUG
		printf( "%s : COM really UNinitialized\n", __FUNCTION__);
	#endif
	}

	COMinitialized--;
}

//-------------------------------------------------------------------

HRESULT CameraDSHOW::FindCaptureDevice(IBaseFilter ** ppSrcFilter, int number)
{
    HRESULT hr = S_OK;
    IBaseFilter * pSrc = NULL;
    IMoniker* pMoniker =NULL;
    ICreateDevEnum *pDevEnum =NULL;
    IEnumMoniker *pClassEnum = NULL;

    if (!ppSrcFilter)
	{
        return E_POINTER;
	}
	//printf( "ici CameraDSHOW::FindCaptureDevice( %u, %d)\n", ppSrcFilter, number);
   
    // Create the system device enumerator
    hr = CoCreateInstance (CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC,
                           IID_ICreateDevEnum, (void **) &pDevEnum);
    if (FAILED(hr))
    {
        fprintf( stderr, "Couldn't create system enumerator.\n");
    }

    // Create an enumerator for the video capture devices

	if (SUCCEEDED(hr))
	{
	    hr = pDevEnum->CreateClassEnumerator (CLSID_VideoInputDeviceCategory, &pClassEnum, 0);
		if (FAILED(hr))
		{
			fprintf( stderr, "Couldn't create class enumerator ! \n");
	    }
	}

	if (SUCCEEDED(hr))
	{
		// If there are no enumerators for the requested type, then 
		// CreateClassEnumerator will succeed, but pClassEnum will be NULL.
		if (pClassEnum == NULL)
		{
			fprintf( stderr, "No video capture device was detected.\n");
			hr = E_FAIL;
		}
	}

    // Use the first video capture device on the device list.
    // Note that if the Next() call succeeds but there are no monikers,
    // it will return S_FALSE (which is not a failure).  Therefore, we
    // check that the return code is S_OK instead of using SUCCEEDED() macro.

	if (SUCCEEDED(hr))
	{
		for (int i=0; i<=number; i++)
		{
			hr = pClassEnum->Next (1, &pMoniker, NULL);
			//if (FAILED(hr))
			if ( hr == S_FALSE )
			{
				fprintf( stderr, "Unable to access video capture device %d !\n", i);   
				hr = E_FAIL;
			}
		}
 
	}

	if (SUCCEEDED(hr))
    {
        // Bind Moniker to a filter object
        hr = pMoniker->BindToObject(0,0,IID_IBaseFilter, (void**)&pSrc);
        if (FAILED(hr))
        {
            fprintf( stderr, "Couldn't bind moniker to filter object! \n");
        }
    }

    // Copy the found filter pointer to the output parameter.
	if (SUCCEEDED(hr))
	{
	    *ppSrcFilter = pSrc;
		(*ppSrcFilter)->AddRef();
	}

	if( pSrc != NULL )
		pSrc->Release();
	if( pMoniker != NULL )
		pMoniker->Release();
	if( pDevEnum != NULL )
		pDevEnum->Release();
	if( pClassEnum != NULL )
		pClassEnum->Release();

    return hr;
}

//-------------------------------------------------------------------

HRESULT CameraDSHOW::buildGraph(void)
{
	HRESULT hr;
	hr = CoCreateInstance (CLSID_FilterGraph, NULL, CLSCTX_INPROC,
                           IID_IGraphBuilder, (void **) &g_pGraph);
	if (FAILED(hr))
	{
		fprintf( stderr, "Failed to create Filter Graph Builder\n");
        return hr;
	}

    // Create the capture graph builder
    hr = CoCreateInstance (CLSID_CaptureGraphBuilder2 , NULL, CLSCTX_INPROC,
                           IID_ICaptureGraphBuilder2, (void **) &g_pCapture);
    if (FAILED(hr))
    {
		fprintf( stderr, "Failed to create capture Graph Builder\n");
        return hr;
	}

	 // Obtain interfaces for media control and Video Window
    hr = g_pGraph->QueryInterface(IID_IMediaControl,(LPVOID *) &g_pMC);
    if (FAILED(hr))
    {
		fprintf( stderr, "Failed to get video interfaces !\n");
        return hr;
	}
   
	// Attacher le graphe de filtres au graphe de capture
    hr = g_pCapture->SetFiltergraph(g_pGraph);
    if (FAILED(hr))
    {
        fprintf( stderr, "Failed to set capture filter graph !\n");
        return hr;
    }

    // Ajouter le filtre de capture au graphe.
    hr = g_pGraph->AddFilter(pSrcFilter, L"Video Capture");
    if (FAILED(hr))
    {
        fprintf( stderr, "Couldn't add the capture filter to the graph !\n"); 
        pSrcFilter->Release();
        return hr;
    }

	//créer un SampleGrabber (filtre servant à capturer des frames)
	hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER,IID_IBaseFilter, (void**)&pGrabberF);
	if (FAILED(hr))
    {
        fprintf( stderr, "Echec creation grabber !!\n");
        return hr;
    }

	//Appeler l'interface du SampleGrabber
	hr = pGrabberF->QueryInterface(IID_ISampleGrabber, (void**)&pGrabber);
	if (FAILED(hr))
    {
        fprintf( stderr, "Echec creation Interface grabber!!\n");
        return hr;
    }

	// recupere interface de configuration du flux
	hr = g_pCapture->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, pSrcFilter, IID_IAMStreamConfig, (void **)&pCaptureConfig);
	if (FAILED(hr))
    {
        fprintf( stderr, "Echec creation Interface de configuation du flux!!\n");
        return hr;
    }

	//ajouter le SampleGrabber au graphe
	hr = g_pGraph->AddFilter(pGrabberF, L"grabber");
	if (FAILED(hr))
    {
        fprintf( stderr, "Echec a l'ajout du filtre grabber!!");
        pSrcFilter->Release();
        return hr;
    }
    
	// Render the preview pin on the video capture filter
    // Use this instead of g_pGraph->RenderFile
	
	// creer un graphe de rendu vers un SampleGrabber
    hr = g_pCapture->RenderStream (&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video,
                                   pSrcFilter , NULL, pGrabberF);
    if (FAILED(hr))
    {
        fprintf( stderr, "Couldn't render the video capture stream.\n");
        pSrcFilter->Release();
        return hr;
    }
	
#ifdef _DEBUG
    // Add our graph to the running object table, which will allow
    // the GraphEdit application to "spy" on our graph
	g_dwGraphRegister = 0;
    hr = AddGraphToRot(g_pGraph, &g_dwGraphRegister);
    if (FAILED(hr))
    {
        fprintf( stderr, "Failed to register filter graph with ROT ! \n");
        g_dwGraphRegister = 0;
    }
#endif
	

	//activer le SampleGrabber
	hr = pGrabber->SetBufferSamples(TRUE);
    
	// lancer l'execution du graphe
    hr = g_pMC->Run();
    if (FAILED(hr))
    {
        fprintf( stderr, "Couldn't run the graph!\n");
        return hr;
    }

	//récupération de l'interface de controle des paramètres génériques du filtre de capture de la caméra
	//cette récupération ne marche probablement pas avec toutes les caméras...
	hr = pSrcFilter->QueryInterface(IID_IAMVideoProcAmp, (void**)&pProcAmp);
	if (FAILED(hr))
	{
		fprintf( stderr, "Echec a la creation du controleur de parametres generiques de camera !!\n");
		return -1;
	}

	//récupération de l'interface de controle des paramètres spécifiques du filtre de capture de la caméra
	//cette récupération ne marche probablement pas avec toutes les caméras...
	hr = pSrcFilter->QueryInterface(IID_IAMCameraControl, (void **)&pControlCam);
	if (FAILED(hr))
    {
		pControlCam = NULL;
        fprintf( stderr, "Cette camera ne gere pas les parametres specifiques de camera.\n");
    }

	// to access special parameters of some cameras
	// in particular, fine exposure setting of logitech cameras
	// cf http://www.quickcamteam.net/documentation/how-to/logitech-uvc-driver-public-property-set
	hr = pSrcFilter->QueryInterface( IID_IKsPropertySet, (void **)&pKsPropertySet);
	if (FAILED(hr)) 
	{
		pKsPropertySet = NULL;
        fprintf( stderr, "Cette camera ne gere pas les parametres via IKsPropertySet.\n");
	}

	return S_OK;
}

//-------------------------------------------------------------------

} // namespace apicamera

#endif
