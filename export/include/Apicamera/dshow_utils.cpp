#ifdef D_API_WIN32	// windows only

#pragma include_alias( "dxtrans.h", "qedit.h" )
   #define __IDxtCompositor_INTERFACE_DEFINED__
   #define __IDxtAlphaSetter_INTERFACE_DEFINED__
   #define __IDxtJpeg_INTERFACE_DEFINED__
   #define __IDxtKey_INTERFACE_DEFINED__

#include "dshow_utils.h"

const char* convertMediaSubTypeGUIDToString( GUID& code, std::string& buffer)
{
		// explicite le media sub type (format d'image)
		if ( code == myMEDIASUBTYPE_I420 )
			buffer.assign("MEDIASUBTYPE_I420");
		else if ( code == MEDIASUBTYPE_IYUV )
			buffer.assign("MEDIASUBTYPE_IYUV");
		else if ( code == MEDIASUBTYPE_RGB1 )
			buffer.assign("MEDIASUBTYPE_RGB1");
		else if ( code == MEDIASUBTYPE_RGB24 )
			buffer.assign("MEDIASUBTYPE_RGB24");
		else if ( code == MEDIASUBTYPE_RGB32 )
			buffer.assign("MEDIASUBTYPE_RGB32");
		else if ( code == MEDIASUBTYPE_RGB4 )
			buffer.assign("MEDIASUBTYPE_RGB4");
		else if ( code == MEDIASUBTYPE_RGB555 )
			buffer.assign("MEDIASUBTYPE_RGB555");
		else if ( code == MEDIASUBTYPE_RGB565 )
			buffer.assign("MEDIASUBTYPE_RGB565");
		else if ( code == MEDIASUBTYPE_RGB8 )
			buffer.assign("MEDIASUBTYPE_RGB8");
		else if ( code == MEDIASUBTYPE_UYVY )
			buffer.assign("MEDIASUBTYPE_UYVY");
		//else if ( code == MEDIASUBTYPE_VIDEOIMAGE )
		//	buffer.assign("MEDIASUBTYPE_VIDEOIMAGE");
		else if ( code == MEDIASUBTYPE_YUY2 )
			buffer.assign("MEDIASUBTYPE_YUY2");
		else if ( code == MEDIASUBTYPE_YV12 )
			buffer.assign("MEDIASUBTYPE_YV12");
		else if ( code == MEDIASUBTYPE_YVU9 )
			buffer.assign("MEDIASUBTYPE_YVU9");
		else if ( code == MEDIASUBTYPE_YVYU )
			buffer.assign("MEDIASUBTYPE_YVYU");
		else
		{
			char local_buffer[64];

			sprintf( local_buffer, "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X", 
				code.Data1, code.Data2, code.Data3,
				code.Data4[0], code.Data4[1],
				code.Data4[2], code.Data4[3],
				code.Data4[4], code.Data4[5],
				code.Data4[6], code.Data4[7]);
			buffer.assign( local_buffer );
		}

		return buffer.c_str();
}

void MyFreeMediaType(AM_MEDIA_TYPE& mt)
{
    if (mt.cbFormat != 0)
    {
        CoTaskMemFree((PVOID)mt.pbFormat);
        mt.cbFormat = 0;
        mt.pbFormat = NULL;
    }
    if (mt.pUnk != NULL)
    {
        // Unecessary because pUnk should not be used, but safest.
        mt.pUnk->Release();
        mt.pUnk = NULL;
    }
}

void MyDeleteMediaType(AM_MEDIA_TYPE *pmt)
{
    if (pmt != NULL)
    {
        MyFreeMediaType(*pmt); // See FreeMediaType for the implementation.
        CoTaskMemFree(pmt);
    }
}

HRESULT AddGraphToRot(IUnknown *pUnkGraph, DWORD *pdwRegister) 
{
    IMoniker * pMoniker;
    IRunningObjectTable *pROT;
    WCHAR wsz[128];
    HRESULT hr;

    if (!pUnkGraph || !pdwRegister)
        return E_POINTER;

    if (FAILED(GetRunningObjectTable(0, &pROT)))
        return E_FAIL;

    hr = StringCchPrintfW(wsz, NUMELMS(wsz), L"FilterGraph %08x pid %08x\0", (DWORD_PTR)pUnkGraph, 
              GetCurrentProcessId());

    hr = CreateItemMoniker(L"!", wsz, &pMoniker);
    if (SUCCEEDED(hr)) 
    {
        // Use the ROTFLAGS_REGISTRATIONKEEPSALIVE to ensure a strong reference
        // to the object.  Using this flag will cause the object to remain
        // registered until it is explicitly revoked with the Revoke() method.
        //
        // Not using this flag means that if GraphEdit remotely connects
        // to this graph and then GraphEdit exits, this object registration 
        // will be deleted, causing future attempts by GraphEdit to fail until
        // this application is restarted or until the graph is registered again.
        hr = pROT->Register(ROTFLAGS_REGISTRATIONKEEPSALIVE, pUnkGraph, 
                            pMoniker, pdwRegister);
        pMoniker->Release();
    }

    pROT->Release();
    return hr;
}
#endif
