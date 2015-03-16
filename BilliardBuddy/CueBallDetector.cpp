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

cv::vector<cv::Vec2i> CueBallDetector::detect(cv::Mat frame, int frameIterator)
{
	int cropHeight, cropWidth;
	if (cropY + CROP_HEIGHT <= frame.rows)
	{
		cropHeight = CROP_HEIGHT;
	}
	else
	{
		cropHeight = frame.rows - cropY;
	}

	if (cropX - CROP_WIDTH / 2 <= frame.cols)
	{
		cropWidth = CROP_WIDTH;
	}
	else
	{
		cropWidth = (frame.cols - cropX)/2;
	}

	cv::Mat croppedFrame = frame(cv::Rect(cropX - cropWidth, cropY, cropWidth*2, cropHeight)); // Crop the image for the typical location of the cue.
	detectWithBlobDetector(frame);
	imshow("Debug cueball", croppedFrame);

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
	int iLowH = 0;
	int iHighH = 57;
	int iLowS = 0;
	int iHighS = 245;
	int iLowV = 0;
	int iHighV = 235;

	//Create binary colour segmented mask
	cv::Mat whiteBallMask = hsiSegment(frame, open_size, close_size,
		iLowH, iLowS, iLowV, iHighH, iHighS, iHighV);

	imshow("Cue Ball", whiteBallMask);

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
	blob_detector.detect(whiteBallMask, keypoints);

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
	cv::drawKeypoints(whiteBallMask, keypoints, keypointMask, cv::Scalar::all(-1), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
	cv::Mat maskedKeypointFrame;

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