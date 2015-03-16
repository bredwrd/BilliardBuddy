#include "PoolTableDetector.h"

PoolTableDetector::PoolTableDetector()
{
	ballDetector = BallDetector();
	cueBallDetector = CueBallDetector();
	cv::vector<cv::Vec2i> ballCoordinates = cv::vector<cv::Vec2i>();
}

PoolTableDetector::~PoolTableDetector()
{
}

cv::vector<cv::Vec2i> PoolTableDetector::detect(cv::Mat frame, int frameIterator)
{
	cv::vector<cv::Vec2i> pockets;
	return pockets; // TODO- populate vector containing points of pockets
}

cv::vector<pocket> PoolTableDetector::detectTable(cv::Mat frame, int frameIterator)
{
	pockets = detectTableWithColourSegmentation(frame, frameIterator);
	return pockets; // TODO- populate vector containing points of pockets
}

void PoolTableDetector::setCueMask(cv::Mat& mask)
{
	cueMask = mask;
}

cv::vector<pocket> PoolTableDetector::detectTableWithColourSegmentation(cv::Mat& frame, int frameIterator)
{
	//Can be used to control with trackbars the values
	int iLowH = 55;
	int iHighH = 120;

	int iLowS = 100;
	int iHighS = 255;

	int iLowV = 5;
	int iHighV = 200;

	//Specify opening/closing size
	int open_size = 4;
	int close_size = 4;

	// Downsample before hsi segmentation
	cv::Mat downsampledFrame;

	// HSV segment
	if (frameIterator == 2 || frameIterator == 0)
	{
		cv::resize(frame, downsampledFrame, cv::Size(0, 0), 1, 1, cv::INTER_LINEAR);
		PoolTableDetector::hsiSegment(downsampledFrame, open_size, close_size,
			iLowH, iLowS, iLowV, iHighH, iHighS, iHighV, frameIterator);
	}

	if (frameIterator == 3 || frameIterator == 0)
	{
		// HSV segment
		tableMask = PoolTableDetector::hsiSegment(downsampledFrame, open_size, close_size,
			iLowH, iLowS, iLowV, iHighH, iHighS, iHighV, frameIterator);

		// Upsample after hsi segmentation
		cv::resize(hsiSegmentationStageTwoFrame, hsiSegmentationStageTwoFrame, cv::Size(0, 0), 1, 1, cv::INTER_CUBIC);
	}

	if (frameIterator == 4 || frameIterator == 0)
	{
		// Morphological opening (remove small objects from the foreground)
		erode(hsiSegmentationStageTwoFrame, hsiSegmentationStageTwoFrame, getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(8 * open_size, 8 * open_size)));
		dilate(hsiSegmentationStageTwoFrame, hsiSegmentationStageTwoFrame, getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(16 * open_size, 16 * open_size)));
	}

	if (frameIterator == 5 || frameIterator == 0)
	{
		cv::Mat maskedFrame;
		frame.copyTo(maskedFrame, hsiSegmentationStageTwoFrame);
		ballCoordinates = ballDetector.detect(maskedFrame, frameIterator);

		cueBallDetector.setTableMask(tableMask);
		cueBallDetector.setCueMask(cueMask);
		cueBallDetector.detect(frame, frameIterator);

		
	}

	if (frameIterator == 6 || frameIterator == 0)
	{
		tableEdgeResult = detectTableEdge(frame, frameIterator);
	}

	return tableEdgeResult;
}

cv::vector<pocket> PoolTableDetector::detectTableEdge(cv::Mat& frame, int frameIterator)
{
	// Thin the edge of the pool table colour.
	Canny(tableMask, tableMask, 100, 180, 3, true);

	cv::Mat houghMap;
	tableMask.copyTo(houghMap);

	cv::vector<cv::Vec4i> lines;
	HoughLinesP(houghMap, lines, 1, CV_PI / 180, 10, 10, 10);
	/*for (size_t i = 0; i < lines.size(); i++)
	{
		cv::Vec4i l = lines[i];
		line(houghMap, cv::Point(l[0], l[1]), cv::Point(l[2], l[3]), cv::Scalar(0, 0, 255), 1, CV_AA);
	}
	imshow("Rail Edge Hough", houghMap);*/

	//if (frameIterator == 6 || frameIterator == 0)
	//{
		// Expand pool edge to include some padding to contain the rail.
		dilate(tableMask, tableMask, getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(50, 50)));
	//}

	cv::Mat maskedEdgeFrame;
	frame.copyTo(maskedEdgeFrame, hsiSegmentationStageTwoFrame);
	//imshow("Rail Edge", maskedEdgeFrame);

	// Initialize Pocket Detector
	PocketDetector pocketDetector = PocketDetector();

	// Run pocket detector
	return pocketDetector.detectPockets(maskedEdgeFrame, frameIterator);
}

cv::Mat PoolTableDetector::hsiSegment(cv::Mat frame, int open_size, int close_size, int iLowH, int iLowS, int iLowV,
												int iHighH, int iHighS, int iHighV, int frameIterator){
	
	if (frameIterator == 2 || frameIterator == 0)
	{
		//Convert the captured frame from BGR to HSV
		cv::Mat imgHSV;
		cvtColor(frame, imgHSV, cv::COLOR_BGR2HSV);

		//Create mask
		inRange(imgHSV, cv::Scalar(iLowH, iLowS, iLowV), cv::Scalar(iHighH, iHighS, iHighV), hsiSegmentationStageOneFrame); //Threshold the image

		//morphological opening (remove small objects from the foreground)
		erode(hsiSegmentationStageOneFrame, hsiSegmentationStageOneFrame, getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(open_size, open_size)));
		dilate(hsiSegmentationStageOneFrame, hsiSegmentationStageOneFrame, getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(open_size, open_size)));
	}
	if (frameIterator == 3 || frameIterator == 0)
	{
		//morphological closing (fill small holes in the foreground)
		dilate(hsiSegmentationStageOneFrame, hsiSegmentationStageTwoFrame, getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(close_size, close_size)));
		erode(hsiSegmentationStageTwoFrame, hsiSegmentationStageTwoFrame, getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(close_size, close_size)));
	}


	return hsiSegmentationStageTwoFrame;
}

void PoolTableDetector::probHoughLines(cv::Mat& frame, cv::Mat& houghMap, int threshold, int minLineLength, int maxLineGap) {
	cv::vector<cv::Vec4i> lines;
	HoughLinesP(frame, lines, 1, CV_PI / 180, threshold, minLineLength, maxLineGap);

	for (size_t i = 0; i < lines.size(); i++)
	{
		cv::Vec4i l = lines[i];
		line(houghMap, cv::Point(l[0], l[1]), cv::Point(l[2], l[3]), cv::Scalar(0, 0, 255), 3, CV_AA);
	}
}

void PoolTableDetector::regHoughLines(cv::Mat& frame, cv::Mat& houghMap, int threshold) {
	cv::vector<cv::Vec2f> lines;
	HoughLines(frame, lines, 1, CV_PI / 180, threshold);

	// draw lines
	for (size_t i = 0; i < lines.size(); i++)
	{
		float rho = lines[i][0], theta = lines[i][1];
		cv::Point pt1, pt2;
		double a = cos(theta), b = sin(theta);
		double x0 = a*rho, y0 = b*rho;
		pt1.x = cvRound(x0 + 1000 * (-b));
		pt1.y = cvRound(y0 + 1000 * (a));
		pt2.x = cvRound(x0 - 1000 * (-b));
		pt2.y = cvRound(y0 - 1000 * (a));
		line(houghMap, pt1, pt2, cv::Scalar(0, 0, 255), 3, CV_AA);
	}
}