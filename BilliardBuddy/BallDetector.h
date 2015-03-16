#pragma once

#include "FeatureDetector.h"

#include <iostream>

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d/features2d.hpp>

class BallDetector :
	public virtual billbud::FeatureDetector
{
private:
	cv::vector<cv::Vec2i> ballCoordinates;
	void detectWithBlobDetector(cv::Mat& frame);
	void detectWithHoughCircles(cv::Mat& frame);
	cv::Mat hsiSegment(cv::Mat& frame, int open_size, int close_size, int iLowH, int iLowS, int iLowV,
		int iHighH, int iHighS, int iHighV);
	cv::Mat tableMask;
	cv::Mat cueMask;
	int cropX = 0;
	int cropY = 0;
	static const int CROP_WIDTH = 20;
	static const int CROP_HEIGHT = 20;

public:
	BallDetector();
	~BallDetector();
	cv::vector<cv::Vec2i> detect(cv::Mat frame, int frameIterator); // returns an arbitrary number of ball coordinates in viewspace
	void setCropX(int value);
	void setCropY(int value);
	void setTableMask(cv::Mat frame);
	void setCueMask(cv::Mat mask);
};
