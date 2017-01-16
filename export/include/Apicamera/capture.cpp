#include "capture.h"

#include <stdio.h>


namespace apicamera
{

//-------------------------------------------------------------------

Capture::Capture(Plugin<apicamera::Camera> *_camlib)
{
	camlib = _camlib;
}

//-------------------------------------------------------------------

Capture::~Capture()
{
	close();
}

//-------------------------------------------------------------------

int Capture::open( apicamera::OpenParameters *param, const char *fileName)
{
	if( camlib == NULL )
		return 0;

	// build camera list
	// if camera is a cameraFILE, only one is detected
	printf( "\nStart cameras detection ...\n");
	while( true )
	{
		apicamera::Camera* camera = camlib->create();
		camList.push_back(camera);
		
		if( camList.back() == NULL )
			break;

		if( fileName == NULL )
		{
			// use OpenParameters structure to configure camera
			if( camList.back()->open( camList.size()-1, param) != 0 )
				break;
		}
		else
		{
			// use configuration file to configure camera
			if( camList.back()->open( camList.size()-1, fileName) != 0 )
				break;
		}

		if( strstr( camList.back()->getName(), "FILE")
			|| strstr( camList.back()->getName(), "Kinect") )
			break;
			// only one detection
	};

	if( camList.back() == NULL
		|| ( ! strstr( camList.back()->getName(), "FILE") 
			&& ! strstr( camList.back()->getName(), "Kinect")) )
	{
		// the last element is not valid
		camlib->destroy(camList.back());
		camList.pop_back();
	}
	
	printf( "%u camera(s) detected.\n", (unsigned int) camList.size());

	return camList.size();
}

//-------------------------------------------------------------------

void Capture::close(void)
{
	while( ! camList.empty()  )
	{
		camlib->destroy(camList.back());
		camList.pop_back();
	}
}

//-------------------------------------------------------------------

std::vector<IplImage*> Capture::getFrames(void)
{
	std::vector<IplImage*> pFrameList;
	std::vector<Camera*>::iterator  camIt;

	for( camIt = camList.begin(); camIt != camList.end(); camIt++ )
	{
		if( (*camIt)->isActive() )
			pFrameList.push_back( (*camIt)->get1Frame() );
	}

	return pFrameList;
}

//-------------------------------------------------------------------

IplImage* Capture::getFrameFromCam(unsigned int camNum)
{
	if( camNum < 0 || camNum >= camList.size() )
		return NULL;

	return camList[camNum]->get1Frame();
}

//-------------------------------------------------------------------

void* Capture::getRawDataPtrFromCam(unsigned int camNum)
{
	if( camNum < 0 || camNum >= camList.size() )
		return NULL;

	return camList[camNum]->getRawDataPtr();
}

//-------------------------------------------------------------------

void Capture::show(void)
{
	std::vector<Camera*>::iterator  camIt;
	std::vector<IplImage*>  pFrameList;
	std::vector<IplImage*>::iterator  frameIt;

	if( getCameraNumber() <= 0 )
	{
		printf( "Capture::show : nothing to show !\n");
		return;
	}

	// create one window per camera
	for( camIt = camList.begin(); camIt != camList.end(); camIt++)
		cvNamedWindow( (*camIt)->getName(), CV_WINDOW_AUTOSIZE );

	// Show the image captured from the camera in the window and repeat
	while( 1 )
	{
		// Get one frame per (active) camera
		pFrameList = getFrames();

		for( camIt = camList.begin(), frameIt = pFrameList.begin();
			camIt != camList.end() && frameIt != pFrameList.end();
			camIt++)
		{
			if( (*camIt)->isActive() )
			{
				cvShowImage( (*camIt)->getName(), (*frameIt));
				frameIt++;
			}
		}

		//If ESC key pressed, Key=0x10001B under OpenCV 0.9.7(linux version),
		//remove higher bits using AND operator
		if( (cvWaitKey(10) & 255) == 27 ) break;
	}

	// Release windows
	for( camIt = camList.begin(); camIt != camList.end(); camIt++)
		cvDestroyWindow( (*camIt)->getName() );
}

//-------------------------------------------------------------------

int Capture::getCameraNumber(void)
{
	//return nbCam;
	return camList.size();
}

//-------------------------------------------------------------------

void Capture::setActiveCam( unsigned int camNum, bool state)
{
	if( camNum < 0 || camNum >= camList.size() )
	{
		fprintf( stderr, "Capture::setActiveCam : invalid camera number %d.\n", camNum);
		return;
	}

	camList[camNum]->setActive(state);
}

//-------------------------------------------------------------------

int Capture::getParameter( unsigned int camNum, int control)
{
	if( camNum < 0 || camNum >= camList.size() )
	{
		fprintf( stderr, "Capture::getParameter : invalid camera number %d.\n", camNum);
		return -1;
	}

    return camList[camNum]->getParameter(control);
}

//-------------------------------------------------------------------

int Capture::setParameter( int control, int value)
{
	std::vector<Camera*>::iterator  camIt;
	int returnValue = 0;

	for( camIt = camList.begin(); camIt != camList.end(); camIt++ )
	{
		if( (*camIt)->setParameter( control, value) == -1 )
		{
		    printf( "Capture::setParameter failed for %s.\n", (*camIt)->getName());
		    returnValue = -1;
		}
	}

	return returnValue;
}

//-------------------------------------------------------------------

int Capture::listParameters( unsigned int camNum)
{
	if( camNum < 0 || camNum >= camList.size() )
	{
		fprintf( stderr, "Capture::listParameters : invalid camera number %d.\n", camNum);
		return -1;
	}

	camList[camNum]->listParameters();

    return 0;
}

//-------------------------------------------------------------------

Camera* Capture::getCameraPtr(unsigned int camNum)
{
	if( camNum < 0 || camNum >= camList.size() )
	{
		fprintf( stderr, "Capture::getCameraPtr : invalid camera number %d.\n", camNum);
		return NULL;
	}

	return camList[camNum];
}

//-------------------------------------------------------------------

} // namespace

