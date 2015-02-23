#pragma once

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/features2d/features2d.hpp"
#include "Pocket.h"

struct Distances {
	double L;
	//X and Y direction ball is travelling in
	cv::Vec2i direction;
};

//Constants for the physics calculations
//Radius of ball in metres
static const double ballRadius = 0.028575;
//Radius of cue ball in metres
static const double cueBallRadius = 0.028575;

class PhysicsModel
{
private:
	cv::vector<cv::Vec2i> ballPoints;

public:
	PhysicsModel();
	cv::vector<cv::Vec2i> calculate(cv::Mat frame, cv::vector<pocket> pockets);
	Distances calculateDistances(cv::vector<pocket> pockets);
	cv::vector<cv::Vec2f> calculatePath();
	~PhysicsModel();
};