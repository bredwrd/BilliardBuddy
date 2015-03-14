#pragma once

#include "FeatureDetector.h"
#include "CueBallDetector.h"
#include <iostream>
#include <stdio.h>

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

class CueDetector :
	public virtual billbud::FeatureDetector
{
private:
	void CueDetector::CannyHoughLineDetect(cv::Mat frame);
	void CueDetector::probHoughLinesCueSegments(cv::Mat& frame);
	static const int HOUGH_THRESHOLD = 30;
	static const int CUE_SEGMENT_MIN_LENGTH = 35;
	static const int CUE_SEGMENT_MAX_GAP = 10;
	static const int HSI_SEGMENTATION_DOWNSAMPLE_FACTOR = 2;
	void CueDetector::mergeCueSegments(cv::Mat& frame);
	void CueDetector::skeleton(cv::Mat& frame);
	void CueDetector::GaussianBlur(cv::Mat& frame);
	void CueDetector::hsiSegment(cv::Mat& frame);
	static const int CROP_X = 30;
	static const int CROP_Y = 280;
	static const int CROP_WIDTH = 420;
	static const int CROP_HEIGHT = 280;
	cv::vector<cv::Vec2i> cueLine;
	cv::vector<cv::Vec2i> cueLineHistory; // Tracks last x (e.g. 5) cueLinePoints and averages them.

public:
	CueDetector();
	cv::vector<cv::Vec2i> detect(cv::Mat frame, int frameIterator);
	~CueDetector();
};

