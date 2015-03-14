#include "BallDetector.h"


BallDetector::BallDetector()
{
}


BallDetector::~BallDetector()
{
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

void BallDetector::detectWithBlobDetector(cv::Mat& frame)
{
	imshow("debug frame", frame);
	//Specify opening/closing size
	int open_size = 5;
	int close_size = 5;

	// Red Balls
	// Hue is on 1 - 25 and 240 - 255 (in GIMP). Use higher range for now and let morphological closing fill it in.
	int iLowH = 170; 
	int iHighH = 255;
	int iLowS = 65;
	int iHighS = 255;
	int iLowV = 100;
	int iHighV = 45;

	//Create binary colour segmented mask
	cv::Mat redBallMask = hsiSegment(frame, open_size, close_size,
		iLowH, iLowS, iLowV, iHighH, iHighS, iHighV);

	// White Balls
	iLowH = 0;
	iHighH = 57;
	iLowS = 0;
	iHighS = 245;
	iLowV = 0;
	iHighV = 235;

	//Create binary colour segmented mask
	cv::Mat whiteBallMask = hsiSegment(frame, open_size, close_size,
		iLowH, iLowS, iLowV, iHighH, iHighS, iHighV);


	cv::Mat maskedFrame;
	cv::Mat allPocketMask = redBallMask + whiteBallMask;
	frame.copyTo(maskedFrame, allPocketMask);
	imshow("All Balls", maskedFrame);

	// set up the parameters (check the defaults in opencv's code in blobdetector.cpp)
	cv::SimpleBlobDetector::Params params;
	params.minDistBetweenBlobs = 50.0f;
	params.filterByInertia = false;
	params.filterByConvexity = false;
	params.filterByColor = false;
	params.filterByCircularity = false;
	params.filterByArea = true;
	params.minArea = 50.0f;
	params.maxArea = 5000.0f;

	// Set up the detector with parameters and detect
	cv::SimpleBlobDetector blob_detector(params);
	cv::vector<cv::KeyPoint> keypoints;
	blob_detector.detect(allPocketMask, keypoints);

	// For changing the vector data type
	int height = int(keypoints.size());
	cv::vector<cv::Vec2i> pocketPoints(height);

	// extract the x y coordinates of the keypoints 
	for (int i = 0; i < keypoints.size(); i++){
		float X = keypoints[i].pt.x;
		float Y = keypoints[i].pt.y;
		//Converts from float to int (doesn't need to worry about negatives)
		pocketPoints[i][0] = int(X + 0.5);
		pocketPoints[i][1] = int(Y + 0.5);

		//Used to Check Pocket Points Conversion is accurate until accuracy can be judged in physics calculations
		/*using std::cout;
		using std::endl;
		cout << "keypoints " << i << " : " << keypoints[i].pt.x << " " << keypoints[i].pt.y << endl;
		cout << "pocket points " << i << " : " << pocketPoints[i][0] << " " << pocketPoints[i][1] << endl;*/
	}

	cv::Mat keypointMask;
	cv::drawKeypoints(allPocketMask, keypoints, keypointMask, cv::Scalar::all(-1), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
	cv::Mat maskedKeypointFrame;
	frame.copyTo(maskedFrame, allPocketMask);
	//imshow("All Pockets w/ Points", keypointMask);
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