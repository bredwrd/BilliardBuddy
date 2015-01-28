#pragma once

#include "FeatureDetector.h"

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

class BallDetector :
	public virtual billbud::FeatureDetector
{
private:
	cv::vector<cv::Vec2i> ballCoordinates;
public:
	BallDetector();
	~BallDetector();
	cv::vector<cv::Vec2i> detect(cv::Mat frame); // returns an arbitrary number of ball coordinates in viewspace
};

