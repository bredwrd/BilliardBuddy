#pragma once

#include "FeatureDetector.h"

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

class CueBallDetector :
	public virtual billbud::FeatureDetector
{
private:
	void CueBallDetector::hsiSegment(cv::Mat& frame);
	void CueBallDetector::gaussianBlur(cv::Mat& frame);
	static const int CROP_X = 30;
	static const int CROP_Y = 320;
	static const int CROP_WIDTH = 420;
	static const int CROP_HEIGHT = 280;
	cv::vector<cv::Vec2i> cueBallCentre;
	int cueBallDiameter; // in pixels

public:
	CueBallDetector();
	cv::vector<cv::Vec2i> detect(cv::Mat frame);
	~CueBallDetector();
};

