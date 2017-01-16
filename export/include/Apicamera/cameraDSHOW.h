#ifndef 	__APICAM_CameraDSHOW_H__
#define 	__APICAM_CameraDSHOW_H__


#ifdef D_API_WIN32	// windows only

#include "camera.h"

#include <dshow.h>		// DirectShow

// les lignes ci-dessous sont indispensables
// sinon erreur à la compilation "error C2283: '<unnamed-tag>" dans qedit.h  
#pragma include_alias( "dxtrans.h", "qedit.h" )
#define __IDxtCompositor_INTERFACE_DEFINED__
#define __IDxtAlphaSetter_INTERFACE_DEFINED__
#define __IDxtJpeg_INTERFACE_DEFINED__
#define __IDxtKey_INTERFACE_DEFINED__
#include <qedit.h>		// for DSHOW ISampleGrabber

#include <string>

#include <Ks.h>
#include <KsMedia.h>
#include <Ksproxy.h>
#include "LVUVCPublic.h"

#ifdef D_BUILDWINDLL
	#define DLL_EXPORT __declspec(dllexport)
#else
	#define DLL_EXPORT
#endif

namespace apicamera
{

/** 
*  \brief Manage a camera with Directshow :( API (Windows only).
*/
class DLL_EXPORT CameraDSHOW : public apicamera::Camera
{
public :
	/**
	*  Class constructor. Make basic initialization.
	*/
	CameraDSHOW();

	/**
	*  Class destructor.
	*/
	~CameraDSHOW();

	/* prevoir un constructeur par copie */

	/**
	*  Open a camera device using Direct Show :(.
	*
	*  \param  numCam  Camera Id, from 0 to n-1.
	*  \param  width  Camera resolution on X axis.
	*  \param  height  Camera resolution on Y axis
	*  \param  fRate  Framerate in fps.
	*  \param  format  Image format, not used for this kind of camera. 
	*
	*  \return 0 if ok, -1 if it failed.
	*/
	virtual int open( int, unsigned int width, unsigned int height, unsigned int fRate,	unsigned int format);

	/**
	*  Open a camera device using Direct Show.
	*
	*  \param  numCam  Camera Id, from 0 to n-1.
	*  \param  param  opening parameters : width, height and fRate must be set.
	*
	*  \return 0 if ok, -1 if it failed.
	*/
	int open( int numCam, OpenParameters *param);

	/**
	*  Close camera device.
	*/
	virtual void close(void);
    
	virtual void* getRawDataPtr(void) { return NULL; }; // not implemented

	/**
	*  Get one frame from the camera.
	*
	*  \return  Pointer to next image from camera.
	*/
	virtual IplImage* get1Frame(void);
	
	/**
	*  Display a list of all parameters.
	*/
	virtual void listParameters(void);

	/**
	*  Get parameter value.
	*
	*  \param  control  Parameter id in E_parameters enum.
	*  \return  parameter value, -1 in case of error.
	*
	*  \bug  hope -1 is not a correct value for a parameter !
	*/
	virtual int getParameter(int control);

	/**
	*  Set parameter value.
	*
	*  \param  control  Parameter id in E_parameters enum.
	*  \param  value  Value to assign to parameter.
	*  \return  0 if successfull, -1 in case of error.
	*/
	virtual int setParameter(int control, int value);

	enum E_parameters 
	{ 
		PARAM_BRIGHTNESS, PARAM_CONTRAST, PARAM_HUE, PARAM_SATURATION, 
		PARAM_SHARPNESS, PARAM_GAMMA, PARAM_COLORENABLE, PARAM_WHITEBALANCE, 
		PARAM_BACKLIGHTCOMPENSATION, PARAM_GAIN, 
		PARAM_GEN_END,
		
		PARAM_PAN, PARAM_TILT, PARAM_ROLL,
		PARAM_ZOOM, PARAM_EXPOSURE, PARAM_IRIS, PARAM_FOCUS, 
		PARAM_SPE_END,
		
		PARAM_LOGITECH_LP1_EXPOSURE_TIME
	};	// PARAM_GEN_END must be the last DShow generic parameter, PARAM_SPE_END must be the last DShow specific parameter

	/**
	*  Convert a CameraDSHOW parameter into character string.
	*
	*  \param  param  Parameter id in E_parameters enum.
	*  \param  buffer  BUffer to write string to.
	*  \return  Pointer to string.
	*/
	const char* convertParameterToString( int param, std::string& buffer);

	/**
	*  Convert a CameraDSHOW parameter into DirecShow parameter code.
	*
	*  \param  param  Parameter id in E_parameters enum.
	*  \return  DirecShow parameter code.
	*/
	long convertParameterToDshowValue( int param);


protected :
	/**
	*  Set camera resolution and framerate.
	*
	*  \param  width  Resolution on X axis.
	*  \param  height  Resolution on Y axis.
	*  \param  fRate  Framerate.
	*  \return  0 if successfull, -1 in case of error.
	*/
	int setResolutionAndFramerate( unsigned int width, unsigned int height, unsigned int fRate);

	/**
	*  Open COM for DirectShow :(
	*
	*  \return  0 if successfull, -1 if error
	*/
	static int openCOM(void);

	/**
	*  Close COM, used by DirectShow :(
	*/
	static void closeCOM(void);

	static unsigned int COMinitialized;	///< COM state for Windows DSHOW :( - Count of cameraDSHOW instances

	// Direct Show Delirium
	HRESULT FindCaptureDevice(IBaseFilter ** ppSrcFilter, int number);
	HRESULT buildGraph(void);

	IBaseFilter *pSrcFilter;
	IGraphBuilder *g_pGraph;
	ICaptureGraphBuilder2 *g_pCapture;
	IMediaControl *g_pMC;
	IBaseFilter *pGrabberF;
	ISampleGrabber *pGrabber;
	IAMStreamConfig *pCaptureConfig;		// pour config resolution/framerate
	IAMVideoProcAmp *pProcAmp;				// pour controle parametres generiques video
	IAMCameraControl *pControlCam;			// pour controle parametres spécifiques cameras
	IKsPropertySet *pKsPropertySet;			// pour controle parametres encore + spécifiques cameras "Sets properties on a kernel-mode filter"
	DWORD g_dwGraphRegister;				// pour connexion au graphe avec graphedit
};

} // namespace apicamera

#endif		// windows only

#endif 	    // __APICAM_CameraDSHOW_H__
