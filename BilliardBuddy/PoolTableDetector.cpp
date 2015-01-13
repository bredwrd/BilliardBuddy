#include "PoolTableDetector.h"

PoolTableDetector::PoolTableDetector()
{
}

PoolTableDetector::~PoolTableDetector()
{
}

void PoolTableDetector::detect(cv::Mat frame)
{
	detectWithColourSegmentation(frame);
	detectWithLineDetection(frame);

	//Detect pink balloons
	detectPocketsWithColourSegmentation(frame);
}

void PoolTableDetector::detectWithColourSegmentation(cv::Mat frame)
{
	//Can be used to control with trackbars the values
	int iLowH = 38;
	int iHighH = 130;

	//Saturation Range from 0-255
	int iLowS = 40;
	int iHighS = 200;

	//Intensity (Value) Range from 0-255
	int iLowV = 30;
	int iHighV = 200;

	cv::Mat imgHSV;

	cvtColor(frame, imgHSV, cv::COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV

	cv::Mat imgThresholded;

	inRange(imgHSV, cv::Scalar(iLowH, iLowS, iLowV), cv::Scalar(iHighH, iHighS, iHighV), imgThresholded); //Threshold the image

	//morphological opening (remove small objects from the foreground)
	erode(imgThresholded, imgThresholded, getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(8, 8)));
	dilate(imgThresholded, imgThresholded, getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(8, 8)));

	//morphological closing (fill small holes in the foreground)
	dilate(imgThresholded, imgThresholded, getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(8, 8)));
	erode(imgThresholded, imgThresholded, getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(8, 8)));

	//Used to make the mask bigger (For our specific situation we want to make sure the mask
	//includes the whole pool table, having it a bit bigger thant he pool table is not an issue.
	dilate(imgThresholded, imgThresholded, getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(30, 30)));

	imshow("HSI Segmentation View", imgThresholded); //show the thresholded image
	//imshow("Original", frame); //show the original image
}

void PoolTableDetector::detectWithLineDetection(cv::Mat frame)
{
	// detect edges (Canny)
	// Vary threshold1 and threshold2 parameters
	Canny(frame, frame, 100, 180, 3, true);

	// Detect lines (regular HoughLines or probabilistic HoughLinesP)
	// Vary threshold parameter and with HouphLinesP, minLineLength and maxLineGap
	// Uncomment ONE and only ONE of regHoughLines or probHoughLines per view... or else!
	// The supposed advantage of HoughLinesP is much increased speed for slightly decreased accuracy.
	cv::Mat houghMap;
	cvtColor(frame, houghMap, CV_GRAY2BGR);
	//regHoughLines(frame, houghMap, 100);
	probHoughLines(frame, houghMap, 80, 20, 11);

	imshow("Hough View", houghMap);
}

void PoolTableDetector::detectPocketsWithColourSegmentation(cv::Mat frame)
{
	//Note: Ranges have to be adjusted to 0-179 from normal Hue range
	int iLowH = 160;
	int iHighH = 170;

	//Note: Ranges have to be adjusted to 0-255 from normal Saturation and Intensity (Value) ranges
	int iLowS = 0;
	int iHighS = 255;
	int iLowV = 0;
	int iHighV = 255;

	cv::Mat imgHSV;

	cvtColor(frame, imgHSV, cv::COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV

	cv::Mat imgThresholded;

	inRange(imgHSV, cv::Scalar(iLowH, iLowS, iLowV), cv::Scalar(iHighH, iHighS, iHighV), imgThresholded); //Threshold the image

	//morphological opening (remove small objects from the foreground)
	erode(imgThresholded, imgThresholded, getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5)));
	dilate(imgThresholded, imgThresholded, getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5)));

	//morphological closing (fill small holes in the foreground)
	dilate(imgThresholded, imgThresholded, getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(25, 25)));
	erode(imgThresholded, imgThresholded, getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(25, 25)));

	//Used to make the mask bigger (For our specific situation we want to make sure the mask
	//includes the whole pool table, having it a bit bigger thant he pool table is not an issue.
	//dilate(imgThresholded, imgThresholded, getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(30, 30)));

	imshow("HSI Pocket Segmentation View", imgThresholded); //show the thresholded image
	//imshow("Original", frame); //show the original image
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