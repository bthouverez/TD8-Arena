#include "libconfig_addon.h"

//-------------------------------------------------------------------

int openConfigurationFile( config_t *cfg, const char *fileName)
{
	// load configuration file
	config_init(cfg);
	if( ! config_read_file( cfg, fileName)) 
	{
		fprintf( stderr, "Error line %d in configuration file %s : %s\n",
			config_error_line(cfg),
			fileName,
			config_error_text(cfg));
		config_destroy(cfg);
		
		return CONFIG_FALSE; // failed
	}

	return CONFIG_TRUE; // ok
}

//-------------------------------------------------------------------

void closeConfigurationFile(config_t *cfg)
{
	if( cfg != NULL )
		config_destroy(cfg);
}

//-------------------------------------------------------------------

int config_lookup_int32( const config_t *config, const char *path,
	int *value)
{
#ifdef FIX_CONFIG_LOOKUP_INT_132_2
	// ubuntu 10.10 & 12.04
	// config_lookup_int() in libconfig 1.3.2-2 is bugged
	long lvalue;

	if( ! config_lookup_int( config, path, &lvalue) )
		return CONFIG_FALSE;
		
	*value = (int) lvalue;
	
	return CONFIG_TRUE;
#else
	// on win, libconfig 1.4.7 is ok
	// on ubuntu 14.04, libconfig 1.4.9-2 is ok
	return config_lookup_int( config, path, value);
#endif	
}

//-------------------------------------------------------------------

int config_lookup_float32( const config_t *config, const char *path,
	float *value)
{
	// ubuntu 10.10
	// config_lookup_int() in libconfig 1.3.2-2 is bugged
	double dvalue;

	if( ! config_lookup_float( config, path, &dvalue) )
		return CONFIG_FALSE;
		
	*value = (float) dvalue;
	
	return CONFIG_TRUE;
}
  
//-------------------------------------------------------------------

int config_lookup_array_float( const config_t *config, const char *path, int n,
	float *values)
{
	// look for array name
	config_setting_t *array = config_lookup( config, path);
	if( array == NULL )
		return CONFIG_FALSE;
	
	// check array size
	int length = config_setting_length(array);
	//printf( "length = %d\n", length);
	if( length < n )
		return CONFIG_FALSE;

	// read values
	for( int i = 0; i < n && i < length; i++)
		values[i] = config_setting_get_float_elem( array, i);

	return CONFIG_TRUE;
}


