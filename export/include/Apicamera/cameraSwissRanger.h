
#ifndef 	__APICAM_CAMERASWISSRANGER_H__
#define 	__APICAM_CAMERASWISSRANGER_H__


#include "camera.h"

#include <libMesaSR.h>


// Clamp macro (usefull) Must be insert elsewhere...
#define CLAMP(val,min,max) (((val)<(min))?(min):(((val)>(max))?(max):(val)))


namespace apicamera
{

static float Distance [] = {3.75f, 5.f, 7.142857f, 7.5f, 7.894837f, 2.5f, 10.f, 15.f, 5.172414f, 4.838710f, 10.344828f, 9.677419f}; // All distance as computed by SwissRanger. This sould be in Mesa API !


/** 
*  \brief Manage a SwissRanger camera with MesaSR library.
*/
class CameraSwissRanger : public Camera
{
public :
	/**
	*  Class constructor. Make basic initialization.
	*/
	CameraSwissRanger();

	/**
	*  Class destructor.
	*/
	~CameraSwissRanger();

	/** \brief  image type */
	enum IMGTYPE
	{
		// Mesa SR lib doc : 
		// "Usually index is 0 for distance image and 1 for amplitude image."
		// I don't like "Usually", but ...
		IMGTYPE_DISTANCE = 0,		 
		IMGTYPE_AMPLITUDE = 1,
		IMGTYPE_CONFIDENCE = 2,
	};
	
	/** \brief  various camera parameters */
	enum CAMERA_PARAMETERS
	{
		PARAM_FIRST, // must be the first parameter
		
		PARAM_ACQUIREMODE,  		// see SR_SetMode in MesaSR lib doc
		PARAM_TIMEOUT,  			// see SR_SetTimeout in MesaSR lib doc
		PARAM_INTEGRATIONTIME,  	// see SR_SetIntegrationTime in MesaSR lib doc
		PARAM_AMPLITUDETHRESHOLD, 	// see SR_SetAmplitudeThreshold in MesaSR lib doc
		PARAM_MODULATIONFREQUENCY,	// see SR_SetModulationFrequency in MesaSR lib doc
		PARAM_DISTANCEOFFSET, 		// see SR_SetDistanceOffset in MesaSR lib doc
		PARAM_AUTOEXPOSURE_ONOFF, 	// see SR_SetAutoExposure in MesaSR lib doc
		PARAM_AUTOEXPOSURE_MININTTIME,	// see SR_SetAutoExposure in MesaSR lib doc
		PARAM_AUTOEXPOSURE_MAXINTTIME,	// see SR_SetAutoExposure in MesaSR lib doc
		PARAM_AUTOEXPOSURE_PERCENTOVERPOS, // see SR_SetAutoExposure in MesaSR lib doc
		PARAM_AUTOEXPOSURE_DESIREDPOS, 	// see SR_SetAutoExposure in MesaSR lib doc
		PARAM_OUTPUTIMGTYPE,		// kind of image returned by get1Frame and getRawDataPtr
	
		PARAM_LAST	// must be the last parameter
	};	

	/** \brief  auto exposure parameters, see SR_SetAutoExposure in MesaSR lib doc */
	typedef struct
	{
		unsigned char minIntTime;
		unsigned char maxIntTime;
		unsigned char percentOverPos;
		unsigned char desiredPos;
	}  AutoExposure;
	

	/**
	*  Open a SwissRanger camera using MesaSR lib.
	*
	*  \param  numCam  Camera Id, from 0 to n-1.
	*  \param  param  opening parameters : mode and modulationFrq must be set.
	*
	*  \return value : 0 = ok, -1 = error.
	*/
	virtual int open( int, OpenParameters *param);
	virtual int open( int, unsigned int width, unsigned int height, unsigned int fRate,	unsigned int format) { return -1; /* not allowed for this kind of camera */ };
	
	/**
	*  Close camera device. Use MesaSR lib.
	*/
	virtual void close(void);

	/**
	*  \brief Get one frame from the camera.
	*  Returns one image of output image type (see setParameter(PARAM_OUTPUTIMGTYPE,...)).
	*  But all images (amplitude, distance and confidence) are computed. Use them by accessors getDistance(), getConfidence(), getAmplitude().
	*  Images are transformed to BGR. You can have the untransformed image with getRawDataPtr().
	*
	*  \return  Pointer to next image (converted to BGR) from camera. 
	*/
	virtual IplImage* get1Frame(void);

	/**
	*  Get next frame from camera and return a pointer to camera raw data.
	*
	*  \return  Pointer to raw data, depending of output image type parameter, see setParameter(PARAM_OUTPUTIMGTYPE,...). Original raw data pointer is of "void *" type.
	*/
	virtual void* getRawDataPtr(void);
	
	/**
	*  Display a list of all parameters.
	*/
	virtual void listParameters(void);

	/**
	*  Get parameter value.
	*
	*  \param  control  Parameter id, in CAMERA_PARAMETERS enum.
	*  \return  parameter value, -1 in case of error.
	*
	*  \bug  hope -1 is not a correct value for a parameter !
	*/
	virtual int getParameter(int control);

	/**
	*  Set parameter value.
	*
	*  \param  control  Parameter id, in CAMERA_PARAMETERS enum.
	*  \return  parameter value, -1 in case of error.
	*/
	virtual int setParameter(int control, int value);

	unsigned int getWidth(void)  { return getResolution().width; }
	unsigned int getHeight(void)  { return getResolution().height; }
	IplImage* getAmplitude(void)  { return m_pSR4000Ampl; }
	IplImage* getConfidence(void)  { return m_pSR4000Conf; }
	IplImage* getDistance(void)  { return m_pSR4000Dist; }
	float* getXYZCoords(void)  { return m_pXYZCoords; }
	
	/**
	*  Get camera parameter name.
	*
	*  \param  parId  Parameter Id, see CAMERA_PARAMETERS enum.
	*  \param  buffer  Buffer to store character string.
	*  \param  bufSize  Size of buffer.
	*/
	void getCameraParameterName( int parId, char* buffer, int bufSize);
	
	/**
	*  Record last frame(s) to file.
	*/
	virtual void recordFrame(void);

protected :
	CMesaDevice*  m_pSwissRangerCapture;	///< Mesa SwissRanger capture object.
	ImgEntry*  m_pImgEntryArray;		///< Array of captured image from SwissRanger.
	ModulationFrq  m_pFrequency;		///< SwissRanger camera frenquency.

	IplImage*  m_pSR4000Dist;		///< Distance map grabed from SwissRanger.
	IplImage*  m_pSR4000Conf;		///< Confidence map grabed from SwissRanger.
	IplImage*  m_pSR4000Ampl;		///< Amplitude map grabed from SwissRanger.
	IplImage*  m_pSR4000AmplBack;	///< Amplitude map back buffer.

	float*  m_pXYZCoords;			///< Buffer for X,Y,Z coordinates transformed from SwissRanger raw image. For pixel i in [0,n-1], X = m_pXYZCoords[i*3], Y = m_pXYZCoords[i*3+1], Z = m_pXYZCoords[i*3+2]

	float  m_fDmax;			///< Distance max, linked to frenquency. (Max SwissRanger vision field).

	int outputImageType;  ///<  distance, confidence or amplitude image.
	int imgCnt;		///<  count of available images, depends on acquire mode.
	AutoExposure autoExposure;  ///< \brief  auto exposure parameters.
};


/**
*  USB callback to use with usb library (SwissRanger drivers like...)
*/
static inline int LibusbCallback( SRCAM srCam, unsigned int msg, unsigned int param, void* data )
{
	return 0;
	//~ switch( msg )
	//~ {
		//~ case CM_MSG_DISPLAY: // redirects all output to console
		//~ {
			//~ return 0;
		//~ }
		//~ case CM_PROGRESS:
		//~ {
			//~ return 0;
		//~ }
		//~ default:
		//~ {
			//~ //default handling
			//~ return SR_GetDefaultCallback()(0,msg,param,data);
		//~ }
	//~ }
}

} // namespace apicamera

#endif 	    // __APICAM_CAMERASWISSRANGER_H__


//DONE un seul tableau X,Y,Z
//DONE remplacer set/getOutputImageType par set/getParameter
//DONE valgrind sur testcam
//DONE int'egrer nouveau open() avec params non standards dans l'API 
//DONE tester calibrage

