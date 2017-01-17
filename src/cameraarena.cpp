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
}

CameraArena::~CameraArena(){}

void CameraArena::show()
{
    cv::imshow( "Frame", frame );      
}

void CameraArena::get()
{
    camera.read(frame);
}

bool CameraArena::intrinsics(const int w, const int h, const float squareSize, const int nb)
{
    std::vector<std::vector<cv::Point2f> > imagePoints;
    std::vector<cv::Mat> images;
    cv::Size imageSize , boardSize = cv::Size(w,h);
    const cv::Scalar RED(0,0,255), GREEN(0,255,0);
    int cmp = 0;

    while( cmp < nb )
    {
    	camera >> frame;
    	images.push_back(frame);
       	cv::Mat view = frame.clone();
        imageSize = view.size();                	// Input Size
        std::vector<cv::Point2f> pointBuf;               // BUffer for matched chessboard points
        // Find matching chessboard
        bool found = cv::findChessboardCorners( frame, boardSize, pointBuf,
        	CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FAST_CHECK | CV_CALIB_CB_NORMALIZE_IMAGE);
        if (found)
        {
            cv::Mat viewGray;
            cv::cvtColor(view, viewGray, cv::COLOR_BGR2GRAY);
            cv::cornerSubPix( viewGray, pointBuf, cv::Size(11,11), cv::Size(-1,-1), 
            	cv::TermCriteria( CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 30, 0.1 ));
            // Save the matched points
            imagePoints.push_back(pointBuf);
            // Draw the corners on the input
            cv::drawChessboardCorners( view, boardSize, cv::Mat(pointBuf), found );
            frame = view.clone();
        	cmp++;
        	std::cerr << cmp << " frames grabbed." << std::endl;
        }
        else
        	std::cerr <<"No corners found" << std::endl;

        cv::waitKey(1);
    }
    
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
            corners.push_back(cv::Point3f(float( j*squareSize ), float( i*squareSize ), 0));

    objectPoints.resize(imagePoints.size(),corners);

    std::cout << "Size " << imagePoints.size() << std::endl;
    std::cout << "Size " << imagePoints[0].size() << std::endl;
    std::cout << "Size " << objectPoints.size() << std::endl;
    std::cout << "Size " << objectPoints[0].size() << std::endl;
    std::cout << "Size " << imageSize << std::endl;

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
    std::cout << "Distorsion Matrix : " << K << std::endl;

    return true;
}
