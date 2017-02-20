#include "cameraarena.hpp"

CameraArena::CameraArena(){}
    
CameraArena::CameraArena(const float width, const float height, const float rate, int device)
{
	camera.open( device );

	if( !camera.isOpened() )
	{
		printf( "Failed to init Camera. Exiting ...\n");
		exit(1);
	}

	camera.set(CV_CAP_PROP_FRAME_WIDTH, width);
	camera.set(CV_CAP_PROP_FRAME_HEIGHT, height);
	camera.set(CV_CAP_PROP_FPS, rate);

    A = cv::Mat::eye(3, 3, CV_64F); // Intrinsic Params
    K = cv::Mat::zeros(8, 1, CV_64F); // Extrinsic params
    R = cv::Mat::eye(3, 3, CV_64F); // Intrinsic Params
    T = cv::Mat::zeros(3, 1, CV_64F); // Extrinsic params

}

CameraArena::~CameraArena(){}

void CameraArena::show()
{
    cv::imshow( "Frame", frame );      
}

void CameraArena::get()
{
    camera >> frame;
}

bool CameraArena::extrinsics(const int w, const int h, const float s)
{
	cv::Mat rvec,tvec;
	cv::Mat view = frame.clone();

    cv::Size imageSize, boardSize = cv::Size(w-1,h-1);
    imageSize = view.size();
    std::vector<cv::Point2f> pointBuf;

	bool found = cv::findChessboardCorners( view, boardSize, pointBuf,
        	CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FAST_CHECK | CV_CALIB_CB_NORMALIZE_IMAGE);
    if (found)
    {
        cv::Mat viewGray;
        cv::cvtColor(view, viewGray, cv::COLOR_BGR2GRAY);
        cv::cornerSubPix( viewGray, pointBuf, cv::Size(11,11), cv::Size(-1,-1), 
    	cv::TermCriteria( CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 100, 0.1 ));
        // Draw the corners on the input
        cv::drawChessboardCorners( frame, boardSize, cv::Mat(pointBuf), found);

	    std::vector<cv::Point3f> corners;
	    // 3D projection of the chessboard
	    for( int i = 0; i < boardSize.height; ++i )
        for( int j = 0; j < boardSize.width; ++j )
		{	 
	        corners.push_back(cv::Point3f(float( j*s ), float( i*s ), 0));
		}

		bool ext = cv::solvePnP(corners, pointBuf, A, K, rvec, tvec);

		if( ext )
		{
			T = tvec.clone();
    		cv::Rodrigues(rvec,R);
			/*std::cout << "extrinsics parameters : " << std::endl;
			std::cout << R << std::endl;
			std::cout << T << std::endl;
        	double x = T.at<double>(0)/10.;
        	double y = T.at<double>(1)/10.;
        	double z = T.at<double>(2)/10.;
        	double dist = sqrt(x*x+y*y+z*z);
			std::cout << "Distance = " << dist << std::endl;*/
		}

		return ext;
    }
    else
    {
    	std::cerr <<"No corners found" << std::endl;
    }

    // GTOC Matrix
    gtoc_matrix[0][0] = R.at<double>(0,0);
    gtoc_matrix[1][0] = R.at<double>(1,0);
    gtoc_matrix[2][0] = R.at<double>(2,0);
    gtoc_matrix[3][0] = 0.0f;

    gtoc_matrix[0][1] = R.at<double>(0,1);
    gtoc_matrix[1][1] = R.at<double>(1,1);
    gtoc_matrix[2][1] = R.at<double>(2,1);
    gtoc_matrix[3][1] = 0.0f;

    gtoc_matrix[0][2] = R.at<double>(0,2);
    gtoc_matrix[1][2] = R.at<double>(1,2);
    gtoc_matrix[2][2] = R.at<double>(2,2);
    gtoc_matrix[3][2] = 0.0f;

    gtoc_matrix[0][3] = T.at<double>(0);
    gtoc_matrix[1][3] = T.at<double>(1);
    gtoc_matrix[2][3] = T.at<double>(2);
    gtoc_matrix[3][3] = 1.0f;

    gtoc_matrix = glm::transpose(gtoc_matrix);

    return found;
}

bool CameraArena::intrinsics(const int w, const int h, const float s, const int nb)
{
    std::vector<std::vector<cv::Point2f> > imagePoints;
    cv::Size imageSize, boardSize = cv::Size(w-1,h-1);
    const cv::Scalar RED(0,0,255), GREEN(0,255,0);
    int cmp = 0;

    while( cmp < nb )
    {
    	get();
    	//images.push_back(frame);
       	cv::Mat view = frame.clone();
        imageSize = view.size();                	// Input Size
        std::vector<cv::Point2f> pointBuf;               // BUffer for matched chessboard points
        // Find matching chessboard
        bool found = cv::findChessboardCorners( view, boardSize, pointBuf,
        	CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FAST_CHECK | CV_CALIB_CB_NORMALIZE_IMAGE);
        if (found)
        {
            cv::Mat viewGray;
            cv::cvtColor(view, viewGray, cv::COLOR_BGR2GRAY);
            cv::cornerSubPix( viewGray, pointBuf, cv::Size(11,11), cv::Size(-1,-1), 
            	cv::TermCriteria( CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 100, 0.1 ));
            // Save the matched points
            imagePoints.push_back(pointBuf);
            // Draw the corners on the input
            cv::drawChessboardCorners( view, boardSize, cv::Mat(pointBuf), found );
        	cmp++;
        	std::cerr << cmp << " frames grabbed." << std::endl;
        }
        else
        	std::cerr <<"No corners found" << std::endl;
        cv::waitKey(1);
    }
    std::cout << "Processing calibration ..." << std::endl;
    // Process calibration
    std::vector<cv::Mat> rvecs, tvecs;
    std::vector<float> reprojErrs;
    double totalAvgErr = 0;
    // Init camera and distorsion matrix
    A = cv::Mat::eye(3, 3, CV_64F); // Intrinsic Params
    K = cv::Mat::zeros(8, 1, CV_64F); // Extrinsic params

    std::vector<std::vector<cv::Point3f> > objectPoints;
    std::vector<cv::Point3f> corners;
    // 3D projection of the chessboard
    for( int i = 0; i < boardSize.height; ++i )
        for( int j = 0; j < boardSize.width; ++j )
            corners.push_back(cv::Point3f(float( j*s ), float( i*s ), 0));

    objectPoints.resize(imagePoints.size(),corners);
    //Find intrinsic and extrinsic camera parameters
    double rms = cv::calibrateCamera(objectPoints, imagePoints, imageSize, A,
                                 K, rvecs, tvecs, CV_CALIB_FIX_K4|CV_CALIB_FIX_K5);

    std::cout << "Re-projection error : "<< rms << std::endl;
    bool ok = checkRange(A) && checkRange(K);

    std::vector<cv::Point2f> imagePoints2;
    int i, totalPoints = 0;
    double totalErr = 0, err;
    reprojErrs.resize(objectPoints.size());

    // Compute errors avg
    for( i = 0; i < (int)objectPoints.size(); ++i )
    {
        cv::projectPoints( cv::Mat(objectPoints[i]), rvecs[i], tvecs[i], A,
                       K, imagePoints2);
        err = norm(cv::Mat(imagePoints[i]), cv::Mat(imagePoints2), CV_L2);

        int n = (int)objectPoints[i].size();
        reprojErrs[i] = (float) std::sqrt(err*err/n);
        totalErr        += err*err;
        totalPoints     += n;
    }

    totalAvgErr = std::sqrt(totalErr/totalPoints);

    std::cout << (ok ? "Calibration succeeded" : "Calibration failed")
         << ". average re-projection error = "  << totalAvgErr << std::endl;

    // Remap and optimize view
    cv::Mat view, rview, map1, map2;
    cv::initUndistortRectifyMap(A, K, cv::Mat(),
                            cv::getOptimalNewCameraMatrix(A, K, imageSize, 1, imageSize, 0),
                            imageSize, CV_16SC2, map1, map2);

    std::cout << "Camera Matrix : " << A << std::endl;
    // std::cout << "Distorsion Matrix : " << K << std::endl;
	
    intrinsic_matrix = glm::mat3(   A.at<double>(0,0), A.at<double>(1,0), A.at<double>(2,0),
                                    A.at<double>(0,1), A.at<double>(1,1), A.at<double>(2,1),
                                    A.at<double>(0,2), A.at<double>(1,2), A.at<double>(2,2));
    
    return true;
}

void CameraArena::read(std::string filename)
{
	std::ifstream file( filename.c_str() );
    if( !file ) 
    {
        std::cout << "Can not load parameters file " << std::endl;
    }
    else
    {
    	// Camera matrix
        file >> A.at<double>(0,0) >> A.at<double>(0,1) >> A.at<double>(0,2);
        file >> A.at<double>(1,0) >> A.at<double>(1,1) >> A.at<double>(1,2);
        file >> A.at<double>(2,0) >> A.at<double>(2,1) >> A.at<double>(2,2);
    	// Distorsions matrix
        file >> K.at<double>(0);
        file >> K.at<double>(1);
        file >> K.at<double>(2);
        file >> K.at<double>(3);
        file >> K.at<double>(4);   
    	
    	std::cout << A << std::endl;
    	// std::cout << K << std::endl;

        intrinsic_matrix = glm::mat3(   A.at<double>(0,0), A.at<double>(1,0), A.at<double>(2,0),
                                        A.at<double>(0,1), A.at<double>(1,1), A.at<double>(2,1),
                                        A.at<double>(0,2), A.at<double>(1,2), A.at<double>(2,2));
    }

}    

void CameraArena::write(std::string filename)
{
	std::ofstream file( filename.c_str() );
    if( !file ) 
    {
        std::cout << "Can not load parameters file " << std::endl;
    }
    else
    {
    	// Camera matrix
        file << A.at<double>(0,0) << " " << A.at<double>(0,1) << " " << A.at<double>(0,2) << std::endl;
        file << A.at<double>(1,0) << " " << A.at<double>(1,1) << " " << A.at<double>(1,2) << std::endl;
        file << A.at<double>(2,0) << " " << A.at<double>(2,1) << " " << A.at<double>(2,2) << std::endl;
    	// Distorsions matrix
        file << K.at<double>(0) << " ";
        file << K.at<double>(1) << " ";
        file << K.at<double>(2) << " ";
        file << K.at<double>(3) << " ";
        file << K.at<double>(4) << " " << std::endl;
    }
}


GLuint CameraArena::background()
{
    // Credits 
    // Function from R3dux 
    // http://r3dux.org/2012/01/how-to-convert-an-opencv-cvmat-to-an-opengl-texture/
    cv::Mat tmp;
    cv::flip(frame, tmp, 0);
    GLenum minFilter = GL_NEAREST;
    GLenum magFilter = GL_NEAREST;
    GLenum wrapFilter = GL_CLAMP;
    // Generate a number for our texture's unique handle
    glGenTextures(1, &texture);
    // Bind to our texture handle
    glBindTexture(GL_TEXTURE_2D, texture);
    // Catch silly-mistake texture interpolation method for magnification
    if (magFilter == GL_LINEAR_MIPMAP_LINEAR  ||
        magFilter == GL_LINEAR_MIPMAP_NEAREST ||
        magFilter == GL_NEAREST_MIPMAP_LINEAR ||
        magFilter == GL_NEAREST_MIPMAP_NEAREST)
    {
        magFilter = GL_LINEAR;
    }
    // Set texture interpolation methods for minification and magnification
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
    // Set texture clamping method
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapFilter);
    // Set incoming texture format to:
    // GL_BGR       for CV_CAP_OPENNI_BGR_IMAGE,
    // GL_LUMINANCE for CV_CAP_OPENNI_DISPARITY_MAP,
    // Work out other mappings as required ( there's a list in comments in main() )
    GLenum inputColourFormat = GL_BGR;
    if (tmp.channels() == 1)
    {
        inputColourFormat = GL_LUMINANCE;
    }
    // Create the texture
    glTexImage2D(GL_TEXTURE_2D,     // Type of texture
                 0,                 // Pyramid level (for mip-mapping) - 0 is the top level
                 GL_RGB,            // Internal colour format to convert to
                 tmp.cols,          // Image width  i.e. 640 for Kinect in standard mode
                 tmp.rows,          // Image height i.e. 480 for Kinect in standard mode
                 0,                 // Border width in pixels (can either be 1 or 0)
                 inputColourFormat, // Input image format (i.e. GL_RGB, GL_RGBA, GL_BGR etc.)
                 GL_UNSIGNED_BYTE,  // Image data type
                 tmp.ptr());        // The actual image data itself
    // If we're using mipmaps then generate them. Note: This requires OpenGL 3.0 or higher
    if (minFilter == GL_LINEAR_MIPMAP_LINEAR  ||
        minFilter == GL_LINEAR_MIPMAP_NEAREST ||
        minFilter == GL_NEAREST_MIPMAP_LINEAR ||
        minFilter == GL_NEAREST_MIPMAP_NEAREST)
    {
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    // Keep GL index of texture
    return texture;
}

void CameraArena::release()
{
    glDeleteTextures(1, &texture);
}

glm::vec3 CameraArena::unproject(glm::vec2 point, float sz)
{
    return glm::vec3(glm::inverse(intrinsic_matrix) * glm::vec3(point,1)) * sz;
}

void CameraArena::frustum(int w, int h, float near, float far)
{
    // Frustum faces points
    glm::vec3 mid =     unproject(glm::vec2(w/2.f, h/2.f),near);
    glm::vec3 left =    unproject(glm::vec2(0.f, h/2.f),near);
    glm::vec3 right =   unproject(glm::vec2(w, h/2.f),near);
    glm::vec3 bottom =  unproject(glm::vec2(w/2.f, 0.f),near);
    glm::vec3 top =     unproject(glm::vec2(w/2.f, h),near);
    // Frustum
    float frustum[4];
    frustum[0] = -1.0f * distance(mid, left);
    frustum[1] = 1.0 * distance(mid, right);
    frustum[2] = -1.0f * distance(mid, bottom);
    frustum[3] = 1.0 * distance(mid, top);

    // float a = (frustum[1] + frustum[0]) / (frustum[1] - frustum[0]);
    // float b = (frustum[3] + frustum[2]) / (frustum[3] - frustum[2]);
    // float c = -(far + near) / (far - near);
    // float d = -(2 * far * near) / (far - near);
    // float e = (2 * near) / (frustum[1] - frustum[0]);
    // float f = (2 * near) / (frustum[3] - frustum[2]);

    projection_matrix = glm::frustum(frustum[0], frustum[1], frustum[2], frustum[3], near, far);
    view_matrix = glm::lookAt(glm::vec3(0.f,0.f,0.f),glm::vec3(0.f,0.f,1.f),glm::vec3(0.f,-1.f,0.f));
}