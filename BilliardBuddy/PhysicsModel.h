#pragma once

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/features2d/features2d.hpp"
#include "Pocket.h"
#include "Path.h"

//Constants for the physics calculations
//Radius of ball in metres
//NOTE: TODO - Our snooker balls are smaller than the cue ball. We will have to obtain accurate measurements for these.
static const double ballRadius = 0.028575;
//Radius of cue ball in metres
static const double cueBallRadius = 0.028575;

class PhysicsModel
{
private:
	void collisionModel(cv::vector<Path>& pathVector, cv::Vec2f motionBall,
							cv::vector<cv::Vec2f> ballPoints, cv::vector<cv::Vec2i> cue);

public:
	PhysicsModel();
	cv::vector<Path> calculate(cv::Mat frame, cv::vector<pocket> pockets, cv::vector<cv::Vec2i> cue,
								cv::Vec2f whiteBall, cv::vector<cv::Vec2f> balls);
	~PhysicsModel();
};