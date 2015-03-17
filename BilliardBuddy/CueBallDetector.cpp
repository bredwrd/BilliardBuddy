#include "CueBallDetector.h"

CueBallDetector::CueBallDetector()
{
	cueBallPosition.resize(1);
}

CueBallDetector::~CueBallDetector()
{
}

void CueBallDetector::setCropX(int value)
{
	cropX = value;
}
void CueBallDetector::setCropY(int value)
{
	cropY = value;
}

void CueBallDetector::setTableMask(cv::Mat frame)
{
	tableMask = frame;
}

void CueBallDetector::setCueMask(cv::Mat mask)
{
	cueMask = mask;
	//cv::imshow("cueMask set", mask);
}

cv::vector<cv::Vec2i> CueBallDetector::detect(cv::Mat frame, int frameIterator)
{
		//cv::Mat croppedFrame = frame(cv::Rect(CROP_WIDTH, CROP_HEIGHT, frame.cols - CROP_WIDTH * 2, frame.rows - CROP_HEIGHT * 2)); // Crop the image for the typical location of the cue.
		//imshow("Debug cueball", croppedFrame);

		//erode
		int erode_size = 24;
		erode(tableMask, tableMask, getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(erode_size*2, erode_size*2)));
		
		cv::Mat combinedMask;
		cueMask.copyTo(combinedMask, tableMask);

		cv::Mat tableMaskedFrame;
		frame.copyTo(tableMaskedFrame, combinedMask);
		imshow("debug combined mask", tableMaskedFrame);

		detectWithBlobDetector(tableMaskedFrame);


		// Convert cropped coords to global coords.
		cv::Vec2i cueBall = cv::Vec2i(0 + cropX, 0 + cropY);
		cueBallPosition[0] = cueBall;

	return cueBallPosition;
}

void CueBallDetector::detectWithBlobDetector(cv::Mat& frame)
{
	//Specify opening/closing size
	int open_size = 5;
	int close_size = 5;

	// White Ball
	int iLowH = 14;
	int iHighH = 42;
	int iLowS = 40;
	int iHighS = 245;
	int iLowV = 130;
	int iHighV = 255;

	//Create binary colour segmented mask
	cv::Mat whiteBallMask = hsiSegment(frame, open_size, close_size,
		iLowH, iLowS, iLowV, iHighH, iHighS, iHighV);

	//imshow("Cue Ball", whiteBallMask);

	// Detect centre of mask.
	//cv::Moments m = cv::moments((whiteBallMask >= 50), true);
	//cv::Point2d p(m.m10 / m.m00, m.m01 / m.m00);
	//std::cout <<  "CoM: " << p.x << ", " << p.y << std::endl;

	// set up the parameters (check the defaults in opencv's code in blobdetector.cpp)
	cv::SimpleBlobDetector::Params params;
	params.minDistBetweenBlobs = 50.0f;
	params.filterByInertia = false;
	params.filterByConvexity = false;
	params.filterByColor = false;
	params.filterByCircularity = false;
	params.filterByArea = true;
	params.minArea = 100.0f;
	params.maxArea = 5000.0f;

	// Set up the detector with parameters and detect
	cv::SimpleBlobDetector blob_detector(params);
	cv::vector<cv::KeyPoint> keypoints;
	blob_detector.detect(whiteBallMask, keypoints);

	// For changing the vector data type
	int height = int(keypoints.size());
	cv::vector<cv::Vec2i> pocketPoints(height);


	// extract the x y coordinates of the keypoints 
	for (int i = 0; i < keypoints.size(); i++){
		float X = keypoints[i].pt.x;
		float Y = keypoints[i].pt.y;

		//Used to Check Pocket Points Conversion is accurate until accuracy can be judged in physics calculations
		using std::cout;
		using std::endl;
		cout << "keypoints " << i << " : " << keypoints[i].pt.x << " " << keypoints[i].pt.y << endl;
	}

	cv::Mat keypointMask;
	cv::drawKeypoints(frame, keypoints, keypointMask, cv::Scalar(0,0,0), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
	//imshow("cueball keypoint drawn", keypointMask);
	cv::Mat maskedKeypointFrame;

	if (keypoints.size() > 0)
	{
		cueBallPosition[0][0] = keypoints[0].pt.x;
		cueBallPosition[0][1] = keypoints[0].pt.y;
	}
	//frame.copyTo(maskedFrame, whiteBallMask);
}

cv::Mat CueBallDetector::hsiSegment(cv::Mat& frame, int open_size, int close_size, int iLowH, int iLowS, int iLowV,
	int iHighH, int iHighS, int iHighV) {
	//Convert the captured frame from BGR to HSV
	cv::Mat imgHSV;
	cvtColor(frame, imgHSV, cv::COLOR_BGR2HSV);

	//Create mask
	cv::Mat maskedFrame;
	inRange(imgHSV, cv::Scalar(iLowH, iLowS, iLowV), cv::Scalar(iHighH, iHighS, iHighV), maskedFrame); //Threshold the image

	//morphological opening (remove small objects from the foreground)
	erode(maskedFrame, maskedFrame, getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(open_size, open_size)));
	dilate(maskedFrame, maskedFrame, getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(open_size, open_size)));

	//morphological closing (fill small holes in the foreground)
	dilate(maskedFrame, maskedFrame, getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(close_size, close_size)));
	erode(maskedFrame, maskedFrame, getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(close_size, close_size)));

	return maskedFrame;
}