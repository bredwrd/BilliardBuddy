#pragma once

#include "FeatureDetector.h"

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

public:
	BallDetector();
	~BallDetector();
	cv::vector<cv::Vec2i> detect(cv::Mat frame, int frameIterator); // returns an arbitrary number of ball coordinates in viewspace
};
