#include "kinect_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <pthread.h>

#include "Common/unixtowin.h"

pthread_t freenect_thread;
volatile int die = 0;

pthread_mutex_t gl_backbuf_mutex = PTHREAD_MUTEX_INITIALIZER;

// back: owned by libfreenect (implicit for depth)
// mid: owned by callbacks, "latest frame ready"
// front: owned by GL, "currently being drawn"
uint16_t *depth_mid, *depth_front;
uint8_t *rgb_back, *rgb_mid, *rgb_front;
float *distance_mid, *distance_front;

freenect_context *f_ctx;
freenect_device *f_dev;
int freenect_angle = 0;
int freenect_led;

freenect_video_format requested_format = FREENECT_VIDEO_RGB;
freenect_video_format current_format = FREENECT_VIDEO_RGB;

pthread_cond_t gl_frame_cond = PTHREAD_COND_INITIALIZER;
int got_rgb = 0;
int got_depth = 0;

//uint16_t t_gamma[2048];
float t_meters[2048];

IplImage *rgbImg = NULL; // RGB image
IplImage *irImg = NULL;  // IR image 10 bits
IplImage *irImg8bits = NULL;  // IR image 8 bits
IplImage *depthImg = NULL;  // depth image
IplImage *distImg = NULL;  // distance image

void *freenect_threadfunc(void *arg);
void mallocBuffers(void);
void freeBuffers(void);

/*********************************************************/
/*  interface
/*********************************************************/

bool kinectInit(void)
{
	if( freenect_init(&f_ctx, NULL) < 0 ) 
	{
		printf("freenect_init() failed\n");
		return false;
	}

	//freenect_set_log_level(f_ctx, FREENECT_LOG_DEBUG);
	freenect_set_log_level( f_ctx, FREENECT_LOG_FATAL);
	freenect_select_subdevices( f_ctx,
		(freenect_device_flags)(FREENECT_DEVICE_MOTOR | FREENECT_DEVICE_CAMERA));

	int nr_devices = freenect_num_devices(f_ctx);
	printf("Number of devices found: %d\n", nr_devices);

	int user_device_number = 0;

	if( nr_devices < 1 )
		return false;

	if( freenect_open_device( f_ctx, &f_dev, user_device_number) < 0 ) 
	{
		printf("Could not open device\n");
		return false;
	}

	mallocBuffers();

	int res = pthread_create(&freenect_thread, NULL, freenect_threadfunc, NULL);
	if( res ) 
	{
		printf("pthread_create failed\n");
		freeBuffers();
		return false;
	}
	
	rgbImg = cvCreateImageHeader( cvSize( KINECT_IMG_WIDTH, KINECT_IMG_HEIGHT),
		IPL_DEPTH_8U, 3);
	rgbImg->imageData = (char *) rgb_front;
	
	irImg = cvCreateImageHeader( cvSize( KINECT_IR_WIDTH, KINECT_IR_HEIGHT),
		IPL_DEPTH_16U, 1);
	irImg->imageData = (char *) rgb_front;
	
	irImg8bits = cvCreateImageHeader( 
		cvSize( KINECT_IR_WIDTH, KINECT_IR_HEIGHT), IPL_DEPTH_8U, 1);
	irImg8bits->imageData = (char *) rgb_front;
	
	depthImg = cvCreateImageHeader( 
		cvSize( KINECT_IMG_WIDTH, KINECT_IMG_HEIGHT), IPL_DEPTH_16U, 1);
	depthImg->imageData = (char *) depth_front;
	
	distImg = cvCreateImageHeader( cvSize( KINECT_IMG_WIDTH, KINECT_IMG_HEIGHT),
		IPL_DEPTH_32F, 1);
	distImg->imageData = (char *) distance_front;
	
	for( int i = 0; i < 2048; i++) 
	{
	#if 0
		float v = i/2048.0;
		v = powf(v, 3)* 6;
		t_gamma[i] = v*6*256;
	#endif
		t_meters[i] = rawDepthToMeters(i);
	}

	return true;
}

//---------------------------------------------------------

void kinectClose(void)
{
	die = 1;
	// Not pthread_exit because OSX leaves a thread lying around and doesn't exit
	pthread_join(freenect_thread, NULL);
	freeBuffers();
	cvReleaseImageHeader(&rgbImg);
	cvReleaseImageHeader(&irImg);
	cvReleaseImageHeader(&irImg8bits);
	cvReleaseImageHeader(&depthImg);
	cvReleaseImageHeader(&distImg);
}

//---------------------------------------------------------

float* getDistanceMap(void)
{
	return distance_front;
}

//---------------------------------------------------------

uint8_t* getRgb(void)
{
	return rgb_front;
}

//---------------------------------------------------------

void kinectUpdateData(void)
{
	pthread_mutex_lock(&gl_backbuf_mutex);

	// When using YUV_RGB mode, RGB frames only arrive at 15Hz, so we shouldn't force them to draw in lock-step.
	// However, this is CPU/GPU intensive when we are receiving frames in lockstep.
	if( current_format == FREENECT_VIDEO_YUV_RGB ) 
	{
		while( ! got_depth  &&  ! got_rgb ) 
		{
			pthread_cond_wait( &gl_frame_cond, &gl_backbuf_mutex);
		}
	} 
	else 
	{
		while( (! got_depth || ! got_rgb) && requested_format != current_format)
		{
			//printf( "%s:%d\n", __FILE__, __LINE__);
			pthread_cond_wait( &gl_frame_cond, &gl_backbuf_mutex);
		}
	}

	if( requested_format != current_format ) 
	{
		pthread_mutex_unlock(&gl_backbuf_mutex);
		return;
	}

	uint8_t *tmp;
	uint16_t *tmpdepth;
	float *tmp2;

	if( got_depth ) 
	{
		tmpdepth = depth_front;
		depth_front = depth_mid;
		depth_mid = tmpdepth;
		tmp2 = distance_front;
		distance_front = distance_mid;
		distance_mid = tmp2;
		got_depth = 0;
	
		depthImg->imageData = (char *) depth_front;
		distImg->imageData = (char *) distance_front;
	}
	
	if( got_rgb ) 
	{
		tmp = rgb_front;
		rgb_front = rgb_mid;
		rgb_mid = tmp;
		got_rgb = 0;
		
		rgbImg->imageData = (char *) rgb_front;
		irImg->imageData = (char *) rgb_front;
		irImg8bits->imageData = (char *) rgb_front;
	
		// convert rgb to bgr
		if( current_format == FREENECT_VIDEO_RGB )
			cvCvtColor( rgbImg, rgbImg, CV_RGB2BGR);
	}

	pthread_mutex_unlock(&gl_backbuf_mutex);
}

//---------------------------------------------------------

IplImage* getCvRgb(void)
{
	return rgbImg;
}

//---------------------------------------------------------

inline float fastRawDepthToMeters(int raw_depth)
{
	return t_meters[raw_depth];
}

//---------------------------------------------------------

void mallocBuffers(void)
{
	depth_mid = (uint16_t*) malloc(KINECT_IMG_WIDTH*KINECT_IMG_HEIGHT*2);
	depth_front = (uint16_t*) malloc(KINECT_IMG_WIDTH*KINECT_IMG_HEIGHT*2);
	// RGB buffers are also used for IR image
	rgb_back = (uint8_t*) malloc(KINECT_IR_WIDTH*KINECT_IR_HEIGHT*3);
	rgb_mid = (uint8_t*) malloc(KINECT_IR_WIDTH*KINECT_IR_HEIGHT*3);
	rgb_front = (uint8_t*) malloc(KINECT_IR_WIDTH*KINECT_IR_HEIGHT*3);
	distance_mid = 
		(float*) malloc(KINECT_IMG_WIDTH*KINECT_IMG_HEIGHT*sizeof(float));
	distance_front = 
		(float*) malloc(KINECT_IMG_WIDTH*KINECT_IMG_HEIGHT*sizeof(float));
}

//---------------------------------------------------------

void freeBuffers(void)
{
	free(depth_mid);
	free(depth_front);
	free(rgb_back);
	free(rgb_mid);
	free(rgb_front);
	free(distance_mid);
	free(distance_front);
	depth_mid = NULL;
	depth_front = NULL;
	rgb_back = NULL;
	rgb_mid = NULL;
	rgb_front = NULL;
	distance_mid = NULL;
	distance_front = NULL;
}

//---------------------------------------------------------

float rawDepthToMeters(int raw_depth)
{
	// 1053 <=> ~ 10 m
	// 1069 <=> ~ 20 m
	// 1074 <=> ~ 30 m
	// ...
	// 1084 <=> ~ 533 m
	// 1085 <=> ~ -836 m

	if (raw_depth < 1085)
	{
		return 1.0 / (raw_depth * -0.0030711016 + 3.3309495161);
	}
	
	return 0;
}

//---------------------------------------------------------

IplImage* kinectGetDepthImage(void)
{
	return depthImg;
}

//---------------------------------------------------------

IplImage* kinectGetDistanceImage(void)
{
	return distImg;
}

//---------------------------------------------------------

IplImage* kinectGetIRImage(void)
{
	if( current_format == FREENECT_VIDEO_IR_8BIT )
		return irImg8bits;
	else
		return irImg;
}

//---------------------------------------------------------

IplImage* kinectGetRGBImage(void)
{
	return rgbImg;
}

//---------------------------------------------------------

/*
	Set Kinect into IR mode.
	@param  irFormat  FREENECT_VIDEO_IR_8BIT or FREENECT_VIDEO_IR_10BIT
	@return 0 on success, -1 if error
*/
int kinectSetIRMode(freenect_video_format irFormat)
{
	// thread will change video mode according to requested_format
	requested_format = irFormat;

	// wait for the thread
	while( requested_format != current_format )
		pthread_yield();

	if( current_format == irFormat )
		return 0; // success 
	else
		return -1; // error
}

//---------------------------------------------------------

/*
	Set Kinect into RGB mode.
	@return 0 on success, -1 if error
*/
int kinectSetRGBMode(void)
{
	// thread will change video mode according to requested_format
	requested_format = FREENECT_VIDEO_RGB;

	// wait for the thread
	while( requested_format != current_format )
		pthread_yield();

	if( current_format == FREENECT_VIDEO_RGB )
		return 0; // success 
	else
		return -1; // error
}

//---------------------------------------------------------

/*
	Send low level command to Kinect.
	See http://openkinect.org/wiki/Protocol_Documentation.
*/
int sendKinectCommand(uint16_t reg, uint16_t data, uint8_t adv_cmd)
{
	int ret = freenect_send_cmd(f_dev, reg, data, adv_cmd);

	#ifdef _DEBUG
		printf( "%s(reg=0x%04x, data=0x%04x, adv_cmd=0x%04x) -> 0x%04x\n", __func__, reg, data, adv_cmd, ret);
	#endif

	return ret;
}

//---------------------------------------------------------

/*
	Read Kinect register.
	See http://openkinect.org/wiki/Protocol_Documentation.
*/
uint16_t getKinectValue(uint16_t reg, uint8_t adv_cmd)
{
	uint16_t ret = freenect_get_reg_value(f_dev, reg, adv_cmd);

	#ifdef _DEBUG
		printf( "%s(reg=0x%04x, adv_cmd=0x%04x) -> 0x%04x\n", __func__, reg, adv_cmd, ret);
	#endif

	return ret;
}


/*********************************************************/
/*  KINECT THREAD                                        */
/*********************************************************/

void depth_cb(freenect_device *dev, void *v_depth, uint32_t timestamp)
{
	int i;
	uint16_t *depth = (uint16_t*)v_depth;

	pthread_mutex_lock(&gl_backbuf_mutex);
	for( i = 0; i < 640*480; i++) 
	{
		depth_mid[i] = depth[i];
		/* convert depth code to meters */
		distance_mid[i] = fastRawDepthToMeters(depth[i]);
	}
	got_depth++;
	//printf( "%s:%d\n", __FILE__, __LINE__);
	pthread_cond_signal(&gl_frame_cond);
	pthread_mutex_unlock(&gl_backbuf_mutex);
}

void rgb_cb(freenect_device *dev, void *rgb, uint32_t timestamp)
{
	pthread_mutex_lock(&gl_backbuf_mutex);

	// swap buffers
	assert (rgb_back == rgb);
	rgb_back = rgb_mid;
	freenect_set_video_buffer(dev, rgb_back);
	rgb_mid = (uint8_t*)rgb;

	got_rgb++;
	//printf( "%s:%d\n", __FILE__, __LINE__);
	pthread_cond_signal(&gl_frame_cond);
	pthread_mutex_unlock(&gl_backbuf_mutex);
}

void *freenect_threadfunc(void *arg)
{
	int accelCount = 0;

	//freenect_set_tilt_degs(f_dev,freenect_angle);
	freenect_set_led(f_dev,LED_RED);
	freenect_set_depth_callback(f_dev, depth_cb);
	freenect_set_video_callback(f_dev, rgb_cb);
	freenect_set_video_mode(f_dev, freenect_find_video_mode(FREENECT_RESOLUTION_MEDIUM, current_format));
	freenect_set_depth_mode(f_dev, freenect_find_depth_mode(FREENECT_RESOLUTION_MEDIUM, FREENECT_DEPTH_11BIT));
	freenect_set_video_buffer(f_dev, rgb_back);

	freenect_start_depth(f_dev);
	freenect_start_video(f_dev);

	printf("'w'-tilt up, 's'-level, 'x'-tilt down, '0'-'6'-select LED mode, 'f'-video format\n");

	while (!die /* && freenect_process_events(f_ctx) >= 0 */) 
	{
		freenect_process_events(f_ctx);
		
	#if 0
		//Throttle the text output
		if (accelCount++ >= 2000)
		{
			accelCount = 0;
			freenect_raw_tilt_state* state;
			freenect_update_tilt_state(f_dev);
			state = freenect_get_tilt_state(f_dev);
			double dx,dy,dz;
			freenect_get_mks_accel(state, &dx, &dy, &dz);
			printf("\r raw acceleration: %4d %4d %4d  mks acceleration: %4f %4f %4f", state->accelerometer_x, state->accelerometer_y, state->accelerometer_z, dx, dy, dz);
			fflush(stdout);
		}
	#endif
	
		if (requested_format != current_format) 
		{
			//printf( "%s:%d\n", __FILE__, __LINE__);
			freenect_stop_video(f_dev);
			if( freenect_set_video_mode( f_dev, freenect_find_video_mode( FREENECT_RESOLUTION_MEDIUM, requested_format)) == 0 )
				current_format = requested_format;
			else
			{
				printf( "libfreenect : can't set requested video mode.\n");
				requested_format = current_format;
			}
			freenect_start_video(f_dev);
		}
	}

	printf("\nshutting down streams...\n");

	freenect_stop_depth(f_dev);
	freenect_stop_video(f_dev);

	freenect_close_device(f_dev);
	freenect_shutdown(f_ctx);

	printf("-- done!\n");
	return NULL;
}


