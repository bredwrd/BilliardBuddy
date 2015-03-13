#pragma once

#include "FeatureDetector.h"
#include "PocketDetector.h"
#include "BallDetector.h"
#include "Pocket.h"


#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/features2d/features2d.hpp"
//Not sure if these next 2 need to be included. Builds without but some say errors arise later.
#include "opencv2/nonfree/features2d.hpp"
#include "opencv2/nonfree/nonfree.hpp"
#include <iostream>

class PoolTableDetector :
	public virtual billbud::FeatureDetector
{
private:
	cv::vector<pocket> detectTableWithColourSegmentation(cv::Mat& frame, int frameIterator);
	cv::vector<pocket> detectTableEdge(cv::Mat& frame, cv::Mat& tableMask, int frameIterator);
	void regHoughLines(cv::Mat& view, cv::Mat& houghMap, int threshold);
	void probHoughLines(cv::Mat& view, cv::Mat& houghMap, int threshold, int minLineLength, int maxLineGap);
	cv::Mat hsiSegment(cv::Mat frame, int open_size, int close_size, int iLowH, int iLowS, int iLowV,
		int iHighH, int iHighS, int iHighV);
	cv::vector<pocket> pockets;
	static const int HSI_SEGMENTATION_DOWNSAMPLE_FACTOR = 2;
	BallDetector ballDetector;
	cv::vector<cv::Vec2i> ballCoordinates;

public:
	PoolTableDetector();
	cv::vector<cv::Vec2i> detect(cv::Mat frame, int frameIterator);
	cv::vector<pocket> detectTable(cv::Mat frame, int frameIterator);
	~PoolTableDetector();
};

