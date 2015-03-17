#include "BallDetector.h"


BallDetector::BallDetector()
{
	targetBallPosition.resize(1);
}


BallDetector::~BallDetector()
{
}

void BallDetector::setCropX(int value)
{
	cropX = value;
}
void BallDetector::setCropY(int value)
{
	cropY = value;
}

cv::vector<cv::Vec2i> BallDetector::detectByTargetPocket(cv::Mat frame, int frameIterator, cv::Point2f targetPocket)
{
	if (targetPocket.x - CROP_WIDTH > 0 && (frame.rows - targetPocket.y) - CROP_HEIGHT > 0)
	{
		cv::Mat croppedFrame = frame(cv::Rect(targetPocket.x - CROP_WIDTH, targetPocket.y, CROP_WIDTH * 2, CROP_HEIGHT)); // Crop the image for the typical location of the cue.
		imshow("Debug target ball cropped", croppedFrame);
		detectWithBlobDetector(croppedFrame);

		// Convert cropped coordinates to full-frame coordinates
		targetBallPosition[0][0] = targetBallPosition[0][0] + targetPocket.x - CROP_WIDTH;
		targetBallPosition[0][1] = targetBallPosition[0][1] + targetPocket.y;
	}
	
	return targetBallPosition;
}

void BallDetector::setTableMask(cv::Mat frame)
{
	tableMask = frame;
}

void BallDetector::setCueMask(cv::Mat mask)
{
	cueMask = mask;
	//cv::imshow("cueMask set", mask);
}

cv::vector<cv::Vec2i> BallDetector::detect(cv::Mat frame, int frameIterator)
{
	// Reset cueLines.
	ballCoordinates.clear();

	//detectWithHoughCircles(frame);
	detectWithBlobDetector(frame);

	return ballCoordinates;
}

cv::Mat BallDetector::hsiSegment(cv::Mat& frame, int open_size, int close_size, int iLowH, int iLowS, int iLowV,
	int iHighH, int iHighS, int iHighV){
	//Convert the captured frame from BGR to HSV
	cv::Mat imgHSV;
	cvtColor(frame, imgHSV, cv::COLOR_BGR2HSV);

	// Red Balls
	// We have two hue ranges so segment for both.
	int high_lowH = 162;
	int high_highH = 180;

	cv::Mat highHueMask;
	cv::inRange(imgHSV, cv::Scalar(high_lowH, 0, 0), cv::Scalar(high_highH, 0, 0), highHueMask); //Threshold the image

	//Create mask
	cv::Mat maskedFrame;
	cv::inRange(imgHSV, cv::Scalar(iLowH, iLowS, iLowV), cv::Scalar(iHighH, iHighS, iHighV), maskedFrame); //Threshold the image
	cv::Mat combinedMask;
	combinedMask = maskedFrame + highHueMask;
	cv::Mat maskedBallFrame;
	imgHSV.copyTo(maskedBallFrame, combinedMask);
	cv::imshow("debug combo mask", maskedBallFrame);

	return maskedFrame;
}

void BallDetector::detectWithBlobDetector(cv::Mat& frame)
{
	// Close to filter background.
	int open_size = 5;
	int close_size = 5;

	// Red Balls
	// Hue is on 1 - 25 and 240 - 255 (in GIMP). Use higher range for now and let morphological closing fill it in.
	int iLowH = 0;
	int iHighH = 30;

	int iLowS = 5;
	int iHighS = 225;
	int iLowV = 1;
	int iHighV = 160;

	//Create binary colour segmented mask
	cv::Mat redBallMask = hsiSegment(frame, open_size, close_size,
		iLowH, iLowS, iLowV, iHighH, iHighS, iHighV);

	cv::Mat maskedFrame;
	frame.copyTo(maskedFrame, redBallMask);
	imshow("Red Balls", maskedFrame);

	// set up the parameters (check the defaults in opencv's code in blobdetector.cpp)
	cv::SimpleBlobDetector::Params params;
	params.minDistBetweenBlobs = 50.0f;
	params.filterByInertia = false;
	params.filterByConvexity = false;
	params.filterByColor = false;
	params.filterByCircularity = false;
	params.filterByArea = true;
	params.minArea = 10.0f;
	params.maxArea = 50.0f;

	// Set up the detector with parameters and detect
	cv::SimpleBlobDetector blob_detector(params);
	cv::vector<cv::KeyPoint> keypoints;
	blob_detector.detect(redBallMask, keypoints);

	// For changing the vector data type
	int height = int(keypoints.size());
	cv::vector<cv::Vec2i> pocketPoints(height);

	// extract the x y coordinates of the keypoints 
	for (int i = 0; i < keypoints.size(); i++){
		float X = keypoints[i].pt.x;
		float Y = keypoints[i].pt.y;

		//Used to Check Pocket Points Conversion is accurate until accuracy can be judged in physics calculations
		std::cout << "objball " << i << " : " << keypoints[i].pt.x << " " << keypoints[i].pt.y << std::endl;
	}

	cv::Mat keypointMask;
	cv::drawKeypoints(frame, keypoints, keypointMask, cv::Scalar(0,0,0), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);

	if (keypoints.size() > 0)
	{
		targetBallPosition[0][0] = keypoints[0].pt.x;
		targetBallPosition[0][1] = keypoints[0].pt.y;
	}
}

void BallDetector::detectWithHoughCircles(cv::Mat& frame)
{

	// Convert image to greyscale.
	// Maybe in future take Hue channel instead of greyscale...
	cv::Mat greyscaleFrame;
	cv::cvtColor(frame, greyscaleFrame, CV_BGR2GRAY);

	// Detect circles.
	cv::vector<cv::Vec3f> ballCandidates;
	HoughCircles(greyscaleFrame, ballCandidates, CV_HOUGH_GRADIENT, 1, 4, 60, 30);

	// Draw circles on BGR frame.
	for (size_t i = 0; i < ballCandidates.size(); i++)
	{
		cv::Point center(cvRound(ballCandidates[i][0]), cvRound(ballCandidates[i][1]));
		int radius = cvRound(ballCandidates[i][2]);
		// draw the circle center
		circle(frame, center, 3, cv::Scalar(0, 255, 0), -1, 8, 0);
		// draw the circle outline
		circle(frame, center, radius, cv::Scalar(0, 0, 255), 3, 8, 0);
	}

	//imshow("BallDetector", frame);
}