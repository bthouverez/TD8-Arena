#ifndef 	__APICAM_dshow_utils_H__
#define 	__APICAM_dshow_utils_H__

#ifdef D_API_WIN32	// windows only
#include <dshow.h>		// DirectShow
#include <string>


const GUID myMEDIASUBTYPE_I420 = 
	{ 
		0x30323449, 
		0x0000,
		0x0010, 
		{ 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 }
	};	// missing in uuid.h  :(


const char* convertMediaSubTypeGUIDToString( GUID& code, std::string& buffer);
void MyFreeMediaType(AM_MEDIA_TYPE& mt);
void MyDeleteMediaType(AM_MEDIA_TYPE *pmt);
HRESULT AddGraphToRot(IUnknown *pUnkGraph, DWORD *pdwRegister);

#endif		// windows only

#endif 	    // __APICAM_dshow_utils_H__
