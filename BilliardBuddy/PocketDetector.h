#pragma once

#include "FeatureDetector.h"
#include "PoolTableDetector.h"
#include "PointLocator.h"
#include "Pocket.h"

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

class PocketDetector :
	public virtual billbud::FeatureDetector
{
private:
	cv::Mat PocketDetector::hsiSegment(cv::Mat frame, int open_size, int close_size, int iLowH, int iLowS, int iLowV,
		int iHighH, int iHighS, int iHighV);
	//cv::vector<cv::Vec2i> PocketLine;
	bool defPerspective = false;

public:
	PocketDetector();
	cv::vector<cv::Vec2i> detect(cv::Mat frame, int frameIterator);
	cv::vector<pocket> detectPockets(cv::Mat frame, int frameIterator);
	~PocketDetector();
	bool getDefPerspective();
};

