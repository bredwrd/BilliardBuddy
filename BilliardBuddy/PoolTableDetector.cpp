#include "PoolTableDetector.h"

PoolTableDetector::PoolTableDetector()
{
}

PoolTableDetector::~PoolTableDetector()
{
}

cv::vector<cv::Vec2i> PoolTableDetector::detect(cv::Mat frame)
{
	cv::vector<cv::Vec2i> pockets;
	return pockets; // TODO- populate vector containing points of pockets
}

cv::vector<pocket> PoolTableDetector::detectTable(cv::Mat frame)
{
	pockets = detectTableWithColourSegmentation(frame);
	return pockets; // TODO- populate vector containing points of pockets
}

cv::vector<pocket> PoolTableDetector::detectTableWithColourSegmentation(cv::Mat& frame)
{
	//Can be used to control with trackbars the values
	int iLowH = 55; //Try GIMP converting from 110 if table needs to be tightened up
	int iHighH = 120; //Try GIMP converting from 150 if ^

	int iLowS = 100;
	int iHighS = 255;

	int iLowV = 5;
	int iHighV = 200;

	//Specify opening/closing size
	int open_size = 4;
	int close_size = 4;


	// Downsample before hsi segmentation
	cv::Mat downsampledFrame;
	cv::resize(frame, downsampledFrame, cv::Size(0, 0), 0.125, 0.125, cv::INTER_LINEAR);

	// HSV segment
	cv::Mat tableMask = PoolTableDetector::hsiSegment(downsampledFrame, open_size, close_size,
		iLowH, iLowS, iLowV, iHighH, iHighS, iHighV);

	// Upsample after hsi segmentation
	cv::resize(tableMask, tableMask, cv::Size(0, 0), 8, 8, cv::INTER_CUBIC);

	//morphological opening (remove small objects from the foreground)
	erode(tableMask, tableMask, getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(8*open_size, 8*open_size)));
	dilate(tableMask, tableMask, getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(16*open_size, 16*open_size)));

	cv::Mat maskedFrame;
	frame.copyTo(maskedFrame, tableMask);
	imshow("Masked Pool Table", maskedFrame);

	// Run BallDetector on masked pool table
	BallDetector ballDetector = BallDetector();
	ballDetector.detect(maskedFrame);

	return detectTableEdge(frame, tableMask);
}

cv::vector<pocket> PoolTableDetector::detectTableEdge(cv::Mat& frame, cv::Mat& tableMask)
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

	// Expand pool edge to include some padding to contain the rail.
	dilate(tableMask, tableMask, getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(50, 50)));

	cv::Mat maskedEdgeFrame;
	frame.copyTo(maskedEdgeFrame, tableMask);
	imshow("Rail Edge", maskedEdgeFrame);

	//Initialize Pocket Detector
	PocketDetector pocketDetector = PocketDetector();

	//Run pocket detector
	return pocketDetector.detectPockets(maskedEdgeFrame);
}

cv::Mat PoolTableDetector::hsiSegment(cv::Mat frame, int open_size, int close_size, int iLowH, int iLowS, int iLowV,
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