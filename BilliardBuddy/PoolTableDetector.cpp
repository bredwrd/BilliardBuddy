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
	detectWithLineDetection(frame);
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

	cv::Mat imgThresholded;

	inRange(imgHSV, cv::Scalar(iLowH, iLowS, iLowV), cv::Scalar(iHighH, iHighS, iHighV), imgThresholded); //Threshold the image

	//morphological opening (remove small objects from the foreground)
	// needs more filtering
	erode(imgThresholded, imgThresholded, getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(16, 16)));
	dilate(imgThresholded, imgThresholded, getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(16, 16)));

	//morphological closing (fill small holes in the foreground)
	dilate(imgThresholded, imgThresholded, getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(8, 8)));
	erode(imgThresholded, imgThresholded, getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(8, 8)));

	//Used to make the mask bigger (For our specific situation we want to make sure the mask
	//includes the whole pool table, having it a bit bigger thant he pool table is not an issue.
	dilate(imgThresholded, imgThresholded, getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(30, 30)));

	cv::Mat maskedFrame;
	frame.copyTo(maskedFrame, imgThresholded);
	imshow("Masked Pool Table", maskedFrame);
	imshow("Pool Table Mask", imgThresholded);
}

void PoolTableDetector::detectWithLineDetection(cv::Mat frame)
{
	// detect edges (Canny)
	// Vary threshold1 and threshold2 parameters
	Canny(frame, frame, 100, 180, 3, true);

	// Detect lines (HoughLinesP)
	cv::Mat houghMap;
	cvtColor(frame, houghMap, CV_GRAY2BGR);
	probHoughLines(frame, houghMap, 80, 20, 11);

	imshow("Hough Table View", houghMap);
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