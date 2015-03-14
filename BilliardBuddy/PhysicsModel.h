#pragma once

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/features2d/features2d.hpp"
#include "Pocket.h"
#include "Path.h"
#include "PointLocator.h"

//Constants for the physics calculations
//Radius of ball in metres
//NOTE: TODO - Our snooker balls are smaller than the cue ball. We will have to obtain accurate measurements for these.
static const float ballRadius = 8;//0.028575;
//Radius of cue ball in metres
static const float cueBallRadius = 8;//0.028575;
//Radius of pockets
const float pocketRadius = 11;
//Maximum number of trajectories drawn in one trajectory family
const int maxTotalTrajs = 5;
//Maximum number of edge trajectories drawn 
const int maxEdgeTrajs = 2;
//Maximum distance between cue stick and cue ball for cue ball to be crossed
const int maxCueDist = 60;
//Maximum length of a final trajectory
const float cutFactor = 0.25;
//Pocket location constants
static const cv::Vec2f PT_UR = {xRight, yTop}; 
static const cv::Vec2f PT_UL = {xLeft, yTop}; 
static const cv::Vec2f PT_BL = {xLeft, -yBot};  
static const cv::Vec2f PT_BR = {xRight, -yBot}; 

class PhysicsModel
{
private:
	cv::vector<Path> getTrajectoryGroup(cv::Vec2f param1, cv::Vec2f param2, cv::vector<cv::Vec2f> targetBalls, int ballIndex);
	cv::vector<cv::Vec2f> getContactPointParams(int featureIndex,cv::Vec2f param1, cv::Vec2f param2, cv::vector<cv::Vec2f> targetBalls, int ballIndex, float radius);
	cv::vector<cv::Vec2f> backPointFromLine(int featureIndex, cv::Vec2f param1, cv::Vec2f param2, cv::Vec2f target, float radius);
	cv::vector<cv::Vec2f> edgePointFromLine(int featureIndex, cv::Vec2f param1, cv::Vec2f param2);
	cv::vector<cv::Vec2f> pocketPointFromLine(int featureIndex, cv::Vec2f param1, cv::Vec2f param2, float radius);
	cv::Vec2f frontFromBack(cv::Vec2f backPoint, cv::Vec2f ballCoord);
	cv::Vec2f backFromBack(cv::Vec2f backPoint, cv::Vec2f ballCoord);
public:
	PhysicsModel();
	void calculateTrajectories(cv::vector<Path>& trajectoryPoints, cv::vector<cv::Vec2f> cueBall, cv::vector<cv::Vec2f> targetBalls, cv::vector<cv::Vec2f> cueStick);
	cv::vector<Path> calculate(cv::Mat frame, cv::vector<pocket> pockets, cv::vector<cv::Vec2i> cueStick, cv::vector<cv::Vec2f> cueBall, cv::vector<cv::Vec2f> targetBalls);
	~PhysicsModel();
};