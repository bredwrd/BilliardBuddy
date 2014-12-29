#pragma once

#include "FeatureDetector.h"

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

class CueDetector :
	public virtual billbud::FeatureDetector
{
private:
	void CueDetector::CannyHoughLineDetect(cv::Mat frame);
	void CueDetector::regHoughLines(cv::Mat& frame, int threshold);
	void CueDetector::probHoughLines(cv::Mat& frame, int threshold, int minLength, int maxGap);
	void CueDetector::skeleton(cv::Mat& frame);
	void CueDetector::GaussianBlur(cv::Mat& frame);
	void CueDetector::hsiSegment(cv::Mat& frame);

public:
	CueDetector();
	void detect(cv::Mat frame);
	~CueDetector();
};

