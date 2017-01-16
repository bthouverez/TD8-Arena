#ifndef 	__LIBCONFIG_ADDON_H__
#define 	__LIBCONFIG_ADDON_H__


#include <libconfig.h>

#ifdef D_BUILDWINDLL
	#define DLL_EXPORT __declspec(dllexport)
#else
	#define DLL_EXPORT
#endif


/**
*  Open a configuration file.
*  If successfull, call closeConfigurationFile() to release resources.
*
*  @param  cfg  Pointer to allocated config_t struct.
*  @param  fileName  Configuration file name.
*  @return  CONFIG_TRUE if ok, CONFIG_FALSE on error.
*/
DLL_EXPORT int openConfigurationFile( config_t *cfg, const char *fileName);


/**
*  Close configuration file.
*
*  @param  cfg  Pointer to opened configuration.
*/
DLL_EXPORT void closeConfigurationFile(config_t *cfg);


/**
*  Replacement for bugged config_lookup_int() from libconfig 1.3.2-2.
*  Look at libconfig documentation for more information.
*
*  @param  config  Pointer to opened configuration.
*  @param  path  Setting path in configuration file.
*  @param  value  Place to store setting value.
*  @return  CONFIG_TRUE if ok, CONFIG_FALSE on error.
*/
DLL_EXPORT int config_lookup_int32( const config_t *config, const char *path,
	int *value);

/**
*  Lookup for a single 32bits float.
*
*  @param  config  Pointer to opened configuration.
*  @param  path  Setting path in configuration file.
*  @param  value  Place to store setting value.
*  @return  CONFIG_TRUE if ok, CONFIG_FALSE on error.
*/
DLL_EXPORT int config_lookup_float32( const config_t *config, const char *path,
	float *value);

/**
*  Lookup for an array of floats.
*
*  @param  config  Pointer to opened configuration.
*  @param  path  Path to array in configuration file.
*  @param  n  Count of element to read.
*  @param  values  Pointer to an array of float, to store values.
*  @return CONFIG_TRUE if ok, CONFIG_FALSE on error.
*/
DLL_EXPORT int config_lookup_array_float( const config_t *config, 
	const char *path, int n, float *values);


#endif  // __LIBCONFIG_ADDON_H__
