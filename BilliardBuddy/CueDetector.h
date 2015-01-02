#pragma once

#include "FeatureDetector.h"

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

class CueDetector :
	public virtual billbud::FeatureDetector
{
private:
	void CueDetector::CannyHoughLineDetect(cv::Mat frame);
	void CueDetector::probHoughLinesCueSegments(cv::Mat& frame);
	static const int HOUGH_THRESHOLD = 30;
	static const int CUE_SEGMENT_MIN_LENGTH = 25;
	static const int CUE_SEGMENT_MAX_GAP = 10;
	void CueDetector::mergeCueSegments(cv::Mat& frame);
	void CueDetector::skeleton(cv::Mat& frame);
	void CueDetector::GaussianBlur(cv::Mat& frame);
	void CueDetector::hsiSegment(cv::Mat& frame);
	static const int CROP_X = 30;
	static const int CROP_Y = 320;
	static const int CROP_WIDTH = 420;
	static const int CROP_HEIGHT = 280;
	cv::vector<cv::Vec2i> cueLine;

public:
	CueDetector();
	cv::vector<cv::Vec2i> detect(cv::Mat frame);
	~CueDetector();
};

