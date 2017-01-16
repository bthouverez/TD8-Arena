
#ifndef 	__PLUGIN_H__
#define 	__PLUGIN_H__


#include <stdio.h>

#ifndef D_API_WIN32 // linux
	#include <dlfcn.h>
#else // win
	#include <windows.h>
	#define  __func__       __FUNCTION__ // unix to windows porting
#endif


/**
*  Manage running time class loading (plugin).
*/ 
template <class T> class Plugin
{
public :
	/**
	*  Class constructor. Open library file.
	*
	*  @param  libName  Library name.
	*/
	Plugin(const char *libName);

	/**
	*  Class destructor. Close library file.
	*/
	~Plugin();

	/**
	*  Create new class instance (replace new operator).
	*
	*  @return  Pointer to new class instance.
	*/
	T* create(void);

	/**
	*  Destroy class instance (replace delete operator).
	*
	*  @param  instance  Class instance to delete.
	*/
	void destroy(T *instance);
	
protected :
	// the types of the class factories
	typedef T* create_t(void);
	typedef void destroy_t(T*);

	//! pointer to instance creator
	create_t* createFn;
	
	//! pointer to instance destructor
	destroy_t* destroyFn;
	
	//! library identifier
#ifndef WIN32 
	// linux
	void* lib;
#else 
	// win
	HINSTANCE lib;
#endif
};

//---------------------------------------------------------


//---------------------------------------------------------

template <class T> Plugin<T>::Plugin(const char *libName)
{
	createFn = NULL;
	destroyFn = NULL;

#ifndef WIN32 // linux

	// load library
	//lib = dlopen( libName, RTLD_LAZY | RTLD_DEEPBIND);
	lib = dlopen( libName, RTLD_LAZY);

#if 0 // BUG BUG BUG
/*
	La ligne 
		lib = dlopen( libName, RTLD_LAZY | RTLD_DEEPBIND);
	provoque un crash dans calib en release uniquement :
	
	Start cameras detection ...
*** glibc detected *** ./calib: free(): invalid pointer: 0x00007f2c74dcb260 ***
======= Backtrace: =========
/lib/libc.so.6(+0x774b6)[0x7f2c743334b6]
/lib/libc.so.6(cfree+0x73)[0x7f2c74339c83]
/usr/lib/libstdc++.so.6(_ZNSs9_M_mutateEmmm+0x1e9)[0x7f2c74b75249]
/usr/lib/libstdc++.so.6(_ZNSs15_M_replace_safeEmmPKcm+0x2c)[0x7f2c74b7528c]
../../Apicamera/build/libcameraFILE.so(_ZN9apicamera10CameraFILE18setFileNamePatternEPKc+0x26)[0x7f2c686dfea6]

	Le crash se produit lors de l'appel "fileNamePattern.assign(pattern);" dans "void CameraFILE::setFileNamePattern(const char *pattern)". 
	
	Le bug vient d'une incompatibilité entre listdc++ et l'option RTLD_DEEPBIND de dlopen(). 
	A ce sujet, cf
	
	http://gcc.gnu.org/bugzilla/show_bug.cgi?id=42679
	
	et
	
	http://old.nabble.com/-Bug-libstdc%2B%2B-42679---New%3A-RTLD_DEEPBIND-dlopen-option-for-shared-library-that-uses-libstdc%2B%2B-std%3A%3Aostream-crashes-td27096113.html
*/
#endif

	if( ! lib ) 
	{
	    fprintf( stderr, "Failed to open library %s : %s.\n", libName, 
	    	dlerror());
	    return;
	}

	// reset errors
	dlerror();
	
	// load the symbols
	createFn = (create_t*) dlsym( lib, "create");
	const char* dlsym_error = dlerror();
	if( dlsym_error ) 
	{
	    fprintf( stderr, "Failed to load 'create' symbol in %s library : %s.\n",
	    	libName, dlsym_error);
	    createFn = NULL;
	    return;
	}
	
	destroyFn = (destroy_t*) dlsym( lib, "destroy");
	dlsym_error = dlerror();
	if( dlsym_error ) 
	{
	    fprintf( stderr, "Failed to load 'destroy' symbol in %s library : %s.\n",
	    	libName, dlsym_error);
	    destroyFn = NULL;
	    return;
	}

#else // win

	HINSTANCE lib = LoadLibrary(libName);

	if( lib == NULL )
	{
	    fprintf( stderr, "Failed to open library %s.\n", libName);
		return;
	}

	createFn = (create_t*) GetProcAddress( lib, "create");
	if( createFn == NULL )
	{
	    fprintf( stderr, "Failed to load 'create' symbol in %s library.\n",
	    	libName);
		return;
	}

	destroyFn = (destroy_t*) GetProcAddress( lib, "destroy");
	if( destroyFn == NULL )
	{
	    fprintf( stderr, "Failed to load 'destroy' symbol in %s library.\n",
	    	libName);
		return;
	}

#endif
}

//---------------------------------------------------------

template <class T> Plugin<T>::~Plugin()
{
	if( lib == NULL )
		return;
		
    // unload the library
#ifndef WIN32 
	// linux
    dlclose(lib);
#else 
	// win
    FreeLibrary(lib);
#endif
}

//---------------------------------------------------------

template <class T> T* Plugin<T>::create(void)
{
	if( createFn == NULL || destroyFn == NULL )
		return NULL;
	
    // create an instance of the class
    return createFn();
}

//---------------------------------------------------------

template <class T> void Plugin<T>::destroy(T *cam)
{
	if( cam == NULL || destroyFn == NULL )
		return;
		
    // destroy instance of the class
    destroyFn(cam);
}

//---------------------------------------------------------

#endif // __PLUGIN_H__ 

