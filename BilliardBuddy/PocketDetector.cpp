#include "PocketDetector.h"

PocketDetector::PocketDetector()
{
}

PocketDetector::~PocketDetector()
{
}

cv::vector<cv::Vec2i> PocketDetector::detect(cv::Mat frame, int frameIterator)
{
	cv::vector<cv::Vec2i> pockets;
	return pockets; // TODO- populate vector containing points of pockets
}

cv::vector<pocket> PocketDetector::detectPockets(cv::Mat frame)
{
	//Specify opening/closing size
	int open_size = 5;
	int close_size = 5;

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

	//Orange
	int iLowH = 4; //GIMP converted from 1
	int iHighH = 16; //GIMP converted from 24
	int iLowS = 100;
	int iHighS = 249;
	int iLowV = 92;
	int iHighV = 255;

	//Create binary colour segmented mask
	cv::Mat orangePocketMask = PocketDetector::hsiSegment(frame, open_size, close_size,
		iLowH, iLowS, iLowV, iHighH, iHighS, iHighV);

	// Set up the detector with parameters and detect
	cv::SimpleBlobDetector orangeBlobDetector(params);
	cv::vector<cv::KeyPoint> orangeKeyPoints;
	orangeBlobDetector.detect(orangePocketMask, orangeKeyPoints);

	/*cv::Mat orangeMaskedFrame;
	frame.copyTo(orangeMaskedFrame, orangePocketMask);
	imshow("orange Pockets", orangeMaskedFrame);*/

	//Green
	iLowH = 29; //GIMP converted from 41
	iHighH = 55; //GIMP converted from 79
	iLowS = 50;
	iHighS = 240;
	iLowV = 103;
	iHighV = 255;

	//Create binary colour segmented mask
	cv::Mat greenPocketMask = PocketDetector::hsiSegment(frame, open_size, close_size,
		iLowH, iLowS, iLowV, iHighH, iHighS, iHighV);

	// Set up the detector with parameters and detect
	cv::SimpleBlobDetector greenBlobDetector(params);
	cv::vector<cv::KeyPoint> greenKeyPoints;
	greenBlobDetector.detect(greenPocketMask, greenKeyPoints);

	//Purple
	iLowH = 114; //GIMP converted from 162
	iHighH = 135; //GIMP converted from 193
	iLowS = 100;
	iHighS = 184;
	iLowV = 23;
	iHighV = 82;

	//Create binary colour segmented mask
	cv::Mat purplePocketMask = PocketDetector::hsiSegment(frame, open_size, close_size,
		iLowH, iLowS, iLowV, iHighH, iHighS, iHighV);

	// Set up the detector with parameters and detect
	cv::SimpleBlobDetector purpleBlobDetector(params);
	cv::vector<cv::KeyPoint> purpleKeyPoints;
	purpleBlobDetector.detect(purplePocketMask, purpleKeyPoints);

	//Pink
	iLowH = 165;//GIMP converted from 235
	iHighH = 178;//GIMP converted from 254
	iLowS = 50;
	iHighS = 215;
	iLowV = 120;
	iHighV = 255;

	//Create binary colour segmented mask
	cv::Mat pinkPocketMask = PocketDetector::hsiSegment(frame, open_size, close_size,
		iLowH, iLowS, iLowV, iHighH, iHighS, iHighV);

	// Set up the detector with parameters and detect
	cv::SimpleBlobDetector pinkBlobDetector(params);
	cv::vector<cv::KeyPoint> pinkKeyPoints;
	pinkBlobDetector.detect(pinkPocketMask, pinkKeyPoints);

	/*cv::Mat pinkMaskedFrame;
	frame.copyTo(pinkMaskedFrame, pinkPocketMask);
	imshow("Pink Pockets", pinkMaskedFrame);*/
	
	//Add all masks together
	cv::Mat allPocketMask = pinkPocketMask + greenPocketMask + orangePocketMask + purplePocketMask;
	cv::Mat maskedFrame;
	frame.copyTo(maskedFrame, allPocketMask);
	imshow("All Pockets", maskedFrame);

	// Set up the detector with parameters and detect
	cv::SimpleBlobDetector blob_detector(params);
	cv::vector<cv::KeyPoint> keypoints;
	blob_detector.detect(allPocketMask, keypoints);

	/*// For changing the vector data type
	int height = int(keypoints.size());
	cv::vector<cv::Vec2i> pocketPoints(height);

	// extract the x y coordinates of the keypoints 
	for (int i = 0; i < keypoints.size(); i++){
		float X = keypoints[i].pt.x;
		float Y = keypoints[i].pt.y;
		//Converts from float to int (doesn't need to worry about negatives)
		pocketPoints[i][0] = int(X + 0.5);
		pocketPoints[i][1] = int(Y + 0.5);
	}*/

	cv::Mat keypointMask;
	cv::drawKeypoints(allPocketMask, keypoints, keypointMask, cv::Scalar::all(-1), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
	cv::Mat maskedKeypointFrame;
	frame.copyTo(maskedFrame, allPocketMask);
	imshow("All Pockets w/ Points", keypointMask);

	//Pocket points are filled with inference
	PointLocator pointLocator = PointLocator();
	cv::vector<pocket> pocketPoints;
	pocketPoints = pointLocator.infer(orangeKeyPoints, greenKeyPoints, purpleKeyPoints, pinkKeyPoints);

	return pocketPoints;
}

cv::Mat PocketDetector::hsiSegment(cv::Mat frame, int open_size, int close_size, int iLowH, int iLowS, int iLowV,
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