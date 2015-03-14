#pragma once

#include "FeatureDetector.h"
#include <iostream>
#include <stdio.h>

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d/features2d.hpp>

class CueBallDetector :
	public virtual billbud::FeatureDetector
{
private:
	void CueBallDetector::hsiSegment(cv::Mat& frame);
	int cropX = 0;
	int cropY = 0;
	static const int CROP_WIDTH = 40;
	static const int CROP_HEIGHT = 40;
	cv::vector<cv::Vec2i> cueBallPosition;
	void CueBallDetector::detectWithBlobDetector(cv::Mat& frame);
	cv::Mat CueBallDetector::hsiSegment(cv::Mat& frame, int open_size, int close_size, int iLowH, int iLowS, int iLowV,
		int iHighH, int iHighS, int iHighV);

public:
	CueBallDetector();
	~CueBallDetector();
	cv::vector<cv::Vec2i> CueBallDetector::detect(cv::Mat frame, int frameIterator);
	void setCropX(int value);
	void setCropY(int value);
};

