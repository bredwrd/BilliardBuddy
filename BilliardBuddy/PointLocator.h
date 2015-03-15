#pragma once

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/features2d/features2d.hpp"
#include "Pocket.h"
#include <math.h>
#include <float.h>
#include <iostream>

//Coordinates for Model
static const float xLeft = 0;
static const float xMid = 180;
static const float xRight = 360;
static const float yTop = 0;
static const float yMidTop = 180;
static const float yMid = 360;
static const float yMidBot = 540;
static const float yBot = 720;
//Used for comparing floats
static const float epsilon = 0.005f;

class PointLocator
{
private:
	cv::vector<cv::Vec2i> inferPoint(cv::Mat frame, cv::vector<cv::Vec2i> pockets);
	cv::vector<pocket> reducePoints(cv::vector<pocket> allPockets);
	cv::vector<pocket> labelPockets(cv::vector<cv::KeyPoint> orangeKeyPoints, cv::vector<cv::KeyPoint> greenKeyPoints,
		cv::vector<cv::KeyPoint> purpleKeyPoints, cv::vector<cv::KeyPoint> pinkKeyPoints);
	cv::Vec2f lineEqn(float x1, float y1, float x2, float y2);
	float distPointToLine(float x, float y, cv::Vec2f line);
	float distBetweenKeyPoints(cv::KeyPoint point1, cv::KeyPoint point2);
	void addNonLinearPointLocation(cv::vector<pocket> &pockets);
	void addLastPointLocation(cv::vector<pocket> &pockets, int pocketCount);
	void removePinkCandidate(cv::vector<cv::KeyPoint> &pinkKeyPoints, cv::KeyPoint firstPocket, cv::KeyPoint secondPocket);

public:
	PointLocator();
	cv::vector<pocket> infer(cv::vector<cv::KeyPoint> orangeKeyPoints, cv::vector<cv::KeyPoint> greenKeyPoints,
								cv::vector<cv::KeyPoint> purpleKeyPoints, cv::vector<cv::KeyPoint> pinkKeyPoints);
	~PointLocator();
};