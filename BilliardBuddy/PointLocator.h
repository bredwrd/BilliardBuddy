#pragma once

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/features2d/features2d.hpp"
#include "Pocket.h"

//Coordinates for Model
static const float xLeft = 0;
static const float xMid = 180;
static const float xRight = 360;
static const float yTop = 0;
static const float yMid = 180;
static const float yBot = 360;

class PointLocator
{
private:

	cv::vector<cv::Vec2i> inferPoint(cv::Mat frame, cv::vector<cv::Vec2i> pockets);
	cv::vector<pocket> reducePoints(cv::vector<pocket> allPockets);
	cv::vector<pocket> labelPockets(cv::vector<cv::KeyPoint> orangeKeyPoints, cv::vector<cv::KeyPoint> greenKeyPoints,
		cv::vector<cv::KeyPoint> purpleKeyPoints, cv::vector<cv::KeyPoint> pinkKeyPoints);

public:
	PointLocator();
	cv::vector<pocket> infer(cv::vector<cv::KeyPoint> orangeKeyPoints, cv::vector<cv::KeyPoint> greenKeyPoints,
								cv::vector<cv::KeyPoint> purpleKeyPoints, cv::vector<cv::KeyPoint> pinkKeyPoints);
	~PointLocator();
};