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

	//Specify opening/closing size
	int open_size = 16;
	int close_size = 16;

	//Create binary colour segmented mask
	cv::Mat tableMask = PoolTableDetector::colourSegmentation(frame, open_size, close_size,
		iLowH, iLowS, iLowV, iHighH, iHighS, iHighV);

	cv::Mat maskedFrame;
	frame.copyTo(maskedFrame, tableMask);
	imshow("Masked Pool Table", maskedFrame);

	detectTableEdge(frame, tableMask);
}

void PoolTableDetector::detectPocketsWithColourSegmentation(cv::Mat& frame)
{
	//Specify opening/closing size
	int open_size = 5;
	int close_size = 5;

	//Can be used to control with trackbars the values
	//Orange
	int iLowH = 3; //GIMP converted from 5
	int iHighH = 11; //GIMP converted from 15
	int iLowS = 161;
	int iHighS = 209;
	int iLowV = 124;
	int iHighV = 167;

	//Create binary colour segmented mask
	cv::Mat orangePocketMask = PoolTableDetector::colourSegmentation(frame, open_size, close_size,
		iLowH, iLowS, iLowV, iHighH, iHighS, iHighV);

	//Green
	iLowH = 36; //GIMP converted from 52
	iHighH = 41; //GIMP converted from 58
	iLowS = 150;
	iHighS = 240;
	iLowV = 103;
	iHighV = 177;

	//Create binary colour segmented mask
	cv::Mat greenPocketMask = PoolTableDetector::colourSegmentation(frame, open_size, close_size,
		iLowH, iLowS, iLowV, iHighH, iHighS, iHighV);

	//Purple
	iLowH = 123; //GIMP converted from 175
	iHighH = 133; //GIMP converted from 190
	iLowS = 123;
	iHighS = 184;
	iLowV = 25;
	iHighV = 60;

	//Create binary colour segmented mask
	cv::Mat purplePocketMask = PoolTableDetector::colourSegmentation(frame, open_size, close_size,
		iLowH, iLowS, iLowV, iHighH, iHighS, iHighV);
	
	//Pink
	iLowH = 174;//GIMP converted from 248
	iHighH = 178;//GIMP converted from 254
	iLowS = 155;
	iHighS = 215;
	iLowV = 125;
	iHighV = 201;

	//Create binary colour segmented mask
	cv::Mat pinkPocketMask = PoolTableDetector::colourSegmentation(frame, open_size, close_size, 
												iLowH, iLowS, iLowV, iHighH, iHighS, iHighV);
	
	//Add all masks together
	cv::Mat allPocketMask = pinkPocketMask + purplePocketMask + greenPocketMask + orangePocketMask;
	cv::Mat maskedFrame;
	frame.copyTo(maskedFrame, allPocketMask);
	imshow("All Pockets", maskedFrame);
}

void PoolTableDetector::detectTableEdge(cv::Mat& frame, cv::Mat& tableMask)
{
	// Thin the edge of the pool table colour.
	Canny(tableMask, tableMask, 100, 180, 3, true);

	// Expand pool edge to include some padding to contain the rail.
	dilate(tableMask, tableMask, getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(50, 50)));

	cv::Mat maskedEdgeFrame;
	frame.copyTo(maskedEdgeFrame, tableMask);
	imshow("Rail Edge", maskedEdgeFrame);

	//cv::Mat houghMap;
	//tableMask.copyTo(houghMap);

	//cv::vector<cv::Vec4i> lines;
	//HoughLinesP(tableMask, lines, 1, CV_PI / 180, 300, 100, 20);
	//for (size_t i = 0; i < lines.size(); i++)
	//{
	//	cv::Vec4i l = lines[i];
	//	line(maskedEdgeFrame, cv::Point(l[0], l[1]), cv::Point(l[2], l[3]), cv::Scalar(0, 0, 255), 1, CV_AA);
	//}
	//imshow("Rail Edge Hough", maskedEdgeFrame);

	detectPocketsWithColourSegmentation(maskedEdgeFrame);
}

cv::Mat PoolTableDetector::colourSegmentation(cv::Mat frame, int open_size, int close_size, int iLowH, int iLowS, int iLowV,
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