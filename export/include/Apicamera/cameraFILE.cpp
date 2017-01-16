#include "cameraFILE.h"

#include <highgui.h>


// the class factories

extern "C" DLL_EXPORT apicamera::Camera* create(void)
{
    return new apicamera::CameraFILE;
}

extern "C" DLL_EXPORT void destroy(apicamera::Camera* p) 
{
    delete p;
}


namespace apicamera
{

//-------------------------------------------------------------------

CameraFILE::CameraFILE()
{
	//printf( "ici CameraFILE::CameraFILE()\n");
	indexMin = 0;
	indexMax = 999999;
}

//-------------------------------------------------------------------

CameraFILE::~CameraFILE()
{
	//printf( "ici CameraFILE::~CameraFILE()\n");
	close();
}

//-------------------------------------------------------------------

int CameraFILE::open( int numCam, OpenParameters *param)
{
	setParameter( CameraFILE::PARAM_INDEX_MIN, param->FILEIndexMin);
	if( param->FILEIndexMax > param->FILEIndexMin )
		setParameter( CameraFILE::PARAM_INDEX_MAX, param->FILEIndexMax);
		// setting FILEParam->indexMax = 0 means not setting indexMax at all ...
	setFileNamePattern(param->FILEFileNamePattern.c_str());

	sprintf( name, "FILE-%d", numCam);
	fprintf( stderr, "SUCCESS: %s detected \n", name);
	setActive(true);
	
	return 0;
}

//-------------------------------------------------------------------

void CameraFILE::setFileNamePattern(const char *pattern)
{
	fileNamePattern.assign(pattern);

	// read one frame to set parameters value
	// suppose that all images have the same resolution !
	IplImage* frame = get1Frame();
	nextImageIndex--;
    frameWidth = frame->width;
    frameHeight = frame->height;
    framerate = 0;
    frameFormat = 0;
}

//-------------------------------------------------------------------

void CameraFILE::close(void)
{
	indexMin = 0;
	indexMax = 999999;
}

//-------------------------------------------------------------------

IplImage* CameraFILE::get1Frame(void)
{
	if( lastFrame != NULL )
	{
		cvReleaseImage(&lastFrame);
		lastFrame = NULL;
	}

	if( nextImageIndex < indexMin  ||  nextImageIndex > indexMax )
		nextImageIndex = indexMin;
		
	char fileName[255];

#ifdef D_API_LINUX		
	snprintf( fileName, 255, fileNamePattern.c_str(), nextImageIndex);
#endif
	
#ifdef D_API_WIN32
	_snprintf( fileName, 255, fileNamePattern.c_str(), nextImageIndex);
#endif

#ifdef _DEBUG
	printf( "Loading frame from %s ...\n", fileName);
#endif	
	
#ifdef D_API_LINUX
	// try opening tiff file width 4-channels reader 
	if( strstr( fileName, ".tiff") != NULL )
		lastFrame = load4ChannelsTiffToABGR(fileName);
#endif
	if( lastFrame == NULL )
		lastFrame = cvLoadImage( fileName, CV_LOAD_IMAGE_UNCHANGED);
		
	if( lastFrame == NULL )
		printf("Failed to load image.\n");

	nextImageIndex++;

	if( isUndistortModeOn() && (lastFrame != NULL) )
		undistortFrame(lastFrame);
	
	return  lastFrame;
}

//-------------------------------------------------------------------

int CameraFILE::getParameter(int control)
{
	if( control == PARAM_INDEX_MIN )
    	return (int) indexMin;
    else if( control == PARAM_INDEX_MAX )
    	return (int) indexMax;
    else
    	return -1;
}

//-------------------------------------------------------------------

int CameraFILE::setParameter(int control, int value)
{
	if( control == PARAM_INDEX_MIN )
	{
    	indexMin = (unsigned int) value;
    	nextImageIndex = indexMin;
    }
    else if( control == PARAM_INDEX_MAX )
    {
    	indexMax = (unsigned int) value;
    	nextImageIndex = indexMin;
    }
    else
    	return -1;

   	return 0;
}

//-------------------------------------------------------------------

void CameraFILE::listParameters(void)
{
    printf( "File name pattern : %s\n", fileNamePattern.c_str());
    printf( "Image index min   : %u\n", indexMin);
    printf( "Image index max   : %u\n", indexMax);
}

//-------------------------------------------------------------------

#ifdef D_API_LINUX
IplImage* CameraFILE::load4ChannelsTiffToABGR(const char* filename)
{
	uint32 w, h, nChannels = 0;
	size_t npixels;
	uint32* raster;
	IplImage* dst;
	TIFF* tif = TIFFOpen( filename, "r");
	
	if( ! tif  )
		return NULL;

	TIFFGetField( tif, TIFFTAG_SAMPLESPERPIXEL, &nChannels);
	if( nChannels == 4 )
	{
		TIFFGetField( tif, TIFFTAG_IMAGEWIDTH, &w);
		TIFFGetField( tif, TIFFTAG_IMAGELENGTH, &h);
		dst = cvCreateImage( cvSize(w,h), IPL_DEPTH_8U, 4);
		npixels = w * h;
	
		raster = (uint32*) dst->imageData;
		TIFFReadRGBAImageOriented( tif, w, h, raster, ORIENTATION_TOPLEFT, 0);
	}
	else
		dst = NULL;
		
	TIFFClose(tif);

	return dst;
}
#endif

//-------------------------------------------------------------------

} // namespace apicamera
