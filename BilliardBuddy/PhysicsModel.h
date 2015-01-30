#pragma once

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/features2d/features2d.hpp"

class PhysicsModel
{
private:
	cv::vector<cv::Vec2i> ballPoints;

	//Coordinates for Model
	const float xLeft = 0;
	const float xRight = 480;
	const float yTop = 0;
	const float yBot = 480;

	cv::vector<cv::Vec2i> inferPoint(cv::Mat frame, cv::vector<cv::Vec2i> pockets);

	cv::vector<cv::Vec2i> reducePoints(cv::Mat frame, cv::vector<cv::Vec2i> pockets);

public:
	PhysicsModel();
	cv::vector<cv::Vec2i> calculate(cv::Mat frame, cv::vector<cv::Vec2i> pockets);
	~PhysicsModel();
};