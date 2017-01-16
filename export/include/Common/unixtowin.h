// UNIX to WINDOWS porting facilities

#ifndef 	__UNIX_TO_WIN__
#define 	__UNIX_TO_WIN__

#ifdef WIN32
	// miscellaneous
	#define	  __func__   __FUNCTION__
	#define	  __PRETTY_FUNCTION__   __FUNCTION__
	#define   snprintf   _snprintf
	#define	  sleep(n_seconds)   Sleep(n_seconds*1000)

	// thread
	#include <windows.h>
	#define   pthread_yield   SwitchToThread
#endif

#endif // __UNIX_TO_WIN__



