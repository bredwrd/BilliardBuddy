#pragma once

#include "FeatureDetector.h"

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

class PoolTableDetector :
	public virtual billbud::FeatureDetector
{
private:
	void detectWithColourSegmentation(cv::Mat& frame);
	void detectWithLineDetection(cv::Mat& frame);

	void regHoughLines(cv::Mat& view, cv::Mat& houghMap, int threshold);
	void probHoughLines(cv::Mat& view, cv::Mat& houghMap, int threshold, int minLineLength, int maxLineGap);

public:
	PoolTableDetector();
	void detect(cv::Mat& frame);
	~PoolTableDetector();
};

