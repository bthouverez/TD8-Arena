#ifndef	OBJECTDETECTION_H
#define	OBJECTDETECTION_H

#include <string>
#include <cv.h>

namespace lv
{

/*
 * Structure to store one object detection result.
 */
struct ObjectDetection
{
	cv::Rect rect; // bounding box
	float score;
	int classID;
	std::string className;
};

} // namespace

#endif // OBJECTDETECTION_H



