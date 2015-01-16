#include "PoolTableDetector.h"

PoolTableDetector::PoolTableDetector()
{
}

PoolTableDetector::~PoolTableDetector()
{
}

cv::vector<cv::Vec2i> PoolTableDetector::detect(cv::Mat frame)
{
	detectTableWithColourSegmentation(frame);
	return pockets; // TODO- populate vector containing points of pockets
}

void PoolTableDetector::detectTableWithColourSegmentation(cv::Mat& frame)
{
	//Can be used to control with trackbars the values
	int iLowH = 40;
	int iHighH = 130;

	int iLowS = 40;
	int iHighS = 200;

	int iLowV = 30;
	int iHighV = 200;

	cv::Mat imgHSV;

	cvtColor(frame, imgHSV, cv::COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV

	cv::Mat tableMask;

	inRange(imgHSV, cv::Scalar(iLowH, iLowS, iLowV), cv::Scalar(iHighH, iHighS, iHighV), tableMask); //Threshold the image
	cv::Mat element = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));

	//morphological opening (remove small objects from the foreground)
	// needs more filtering
	erode(tableMask, tableMask, element);
	dilate(tableMask, tableMask, element);

	//morphological closing (fill small holes in the foreground)
	dilate(tableMask, tableMask, element);
	erode(tableMask, tableMask, element);

	// Dilate to include balloon markers.
	//dilate(tableMask, tableMask, getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(50, 50)));


	cv::Mat maskedFrame;
	frame.copyTo(maskedFrame, tableMask);
	//detectTableEdge(frame, tableMask);
	

	detectTableEdge(frame, tableMask);

	CueBallDetector cueBallDetector = CueBallDetector();
	cueBallDetector.detect(maskedFrame);



}

void PoolTableDetector::detectTableEdge(cv::Mat& frame, cv::Mat& tableMask)
{
	// Thin the edge of the pool table colour.
	Canny(tableMask, tableMask, 100, 180, 3, true);

	// Expand pool edge to include some padding to contain the rail.
	dilate(tableMask, tableMask, getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3)));

	cv::Mat maskedEdgeFrame;
	frame.copyTo(maskedEdgeFrame, tableMask);
	//cv::Mat houghMap;
	//tableMask.copyTo(houghMap);

	cv::vector<cv::Vec4i> lines;
	HoughLinesP(tableMask, lines, 1, CV_PI / 180, 150, 30, 15);
	for (size_t i = 0; i < lines.size(); i++)
	{
		cv::Vec4i l = lines[i];
		line(maskedEdgeFrame, cv::Point(l[0], l[1]), cv::Point(l[2], l[3]), cv::Scalar(0, 0, 255), 1, CV_AA);
	}
	imshow("Rail Edge Hough", maskedEdgeFrame);

	imshow("Table Edge", maskedEdgeFrame);
}

void PoolTableDetector::probHoughLines(cv::Mat& frame, cv::Mat& houghMap, int threshold, int minLineLength, int maxLineGap) {
	cv::vector<cv::Vec4i> lines;
	HoughLinesP(frame, lines, 1, CV_PI / 180, threshold, minLineLength, maxLineGap);

	for (size_t i = 0; i < lines.size(); i++)
	{
		cv::Vec4i l = lines[i];
		cv::line(houghMap, cv::Point(l[0], l[1]), cv::Point(l[2], l[3]), cv::Scalar(0, 0, 255), 3, CV_AA);
	}

	imshow("houghmap", houghMap);
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