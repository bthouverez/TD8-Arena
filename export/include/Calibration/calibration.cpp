#include "calibration.h"

#include <stdio.h>
#include <cv.h>
#include <cxcore.h>


Calibration::Calibration(apicamera::Camera *camPtr)
{
	//printf( "ici Calibration::Calibration()\n");
	
	camera = camPtr;
}


Calibration::~Calibration()
{
	//printf( "ici Calibration::~Calibration()\n");
}


void Calibration::saveIntrinsicParameters(const char *fileName)
{
	camera->saveIntrinsicParameters(fileName);
}


void Calibration::saveExtrinsicParameters(const char *fileName)
{
	camera->saveExtrinsicParameters(fileName);
}


void Calibration::loadIntrinsicParameters(const char *fileName)
{
	camera->loadIntrinsicParameters(fileName);
}


void Calibration::loadExtrinsicParameters(const char *fileName)
{
	camera->loadExtrinsicParameters(fileName);
}
