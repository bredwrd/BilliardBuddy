#pragma once

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/features2d/features2d.hpp"
#include "Pocket.h"

class PhysicsModel
{
private:
	cv::vector<cv::Vec2i> ballPoints;

public:
	PhysicsModel();
	cv::vector<cv::Vec2i> calculate(cv::Mat frame, cv::vector<pocket> pockets);
	~PhysicsModel();
};