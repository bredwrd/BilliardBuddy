#include "PhysicsModel.h"

PhysicsModel::PhysicsModel()
{
}

PhysicsModel::~PhysicsModel()
{
}

cv::vector<Path> PhysicsModel::calculate(cv::Mat frame, cv::vector<pocket> pockets, cv::vector<cv::Vec2i> cue,
	cv::Vec2f whiteBall, cv::vector<cv::Vec2f> balls)
{
	//Need 4 pockets to do warp perspective transform
	if (pockets.size() == 4){
		cv::vector<cv::Point2f> sourcePoints(4);
		cv::vector<cv::Point2f> destPoints(4);

		for (int i = 0; i < 4; i++){
			sourcePoints[i] = cv::Point2f(pockets[i].pocketPoints.pt.x, pockets[i].pocketPoints.pt.y);
			destPoints[i] = cv::Point2f(pockets[i].xLocation, pockets[i].yLocation);
		}

		/*// Get mass center
		cv::Point2f center(0, 0);
		for (int i = 0; i < sourcePoints.size(); i++)
			center += sourcePoints[i];

		center *= (1. / sourcePoints.size());
		sortCorners(sourcePoints, center);*/

		//Gets the transformation matrix
		cv::Mat warpMatrix = cv::getPerspectiveTransform(sourcePoints, destPoints);

		cv::Mat rotated;
		cv::Size rSize = { 360, 360 };
		//Performs the warp perspective to obtain the 2D model
		cv::warpPerspective(frame, rotated, warpMatrix, rSize);

		imshow("4 Point Warped Table", rotated);
	}

	//Initialize path vector to be returned
	cv::vector<Path> pathVector;
	
	//Calculates pathVector (a vector filled with start and end points for the visual augmentor)
	collisionModel(pathVector, whiteBall, balls, cue);

	return pathVector;
}

void PhysicsModel::collisionModel(cv::vector<Path>& pathVector, cv::Vec2f motionBall, cv::vector<cv::Vec2f> balls, cv::vector<cv::Vec2i> cue){
	//Initialize collision index to default
	int collisionIndex = -1;

	//Calculate if a ball is hit
	//If a ball is hit return the index as collisionIndex
	//TODO

	//If there is no collision, return
	if (collisionIndex == -1){
		//TODO
		//Add a path to pathVector that ends at some point
		return;
	}

	//Calculate which ball is hit first
	//Update collision index to that ball
	//TODO

	//Calculate collision Point
	cv::Vec2f collisionPoint; //TODO

	//Calculate collision direction vector
	//These should just be a vector, will have to discuss with Brian about why the cue is returned as vector of vectors.
	cv::vector<cv::Vec2i> motionBallDir; //TODO
	cv::vector<cv::Vec2i> newMotionBallDir; //TODO

	//Add collision to path vector
	Path temp;
	temp.startPoint = motionBall;
	temp.endPoint = collisionPoint;
	pathVector.push_back(temp);

	//Create new motion ball to be called recursively and erase the ball that was hit from the stationary balls list
	cv::Vec2f newMotionBall = balls[collisionIndex];
	balls.erase(balls.begin() + collisionIndex);

	//Recursively call collisionModel for white ball
	collisionModel(pathVector, motionBall, balls, motionBallDir);
	//Recursively call collisionModel for target ball
	collisionModel(pathVector, newMotionBall, balls, newMotionBallDir);

	return;
}

/*//TODO
cv::vector<cv::Vec2i> PhysicsModel::reducePoints(cv::Mat frame, cv::vector<cv::Vec2i> pockets)
{
	cv::vector<cv::Vec2i> fourPockets(4);
	fourPockets[0][0] = pockets[0][0];
	fourPockets[0][1] = pockets[0][1];
	fourPockets[1][0] = pockets[1][0];
	fourPockets[1][1] = pockets[1][1];
	fourPockets[2][0] = pockets[2][0];
	fourPockets[2][1] = pockets[2][1];
	fourPockets[3][0] = pockets[3][0];
	fourPockets[3][1] = pockets[3][1];
	return fourPockets;
}

void PhysicsModel::sortCorners(cv::vector<cv::Point2f>& corners, cv::Point2f center)
{
	cv::vector<cv::Point2f> top, bot;

	for (int i = 0; i < corners.size(); i++)
	{
		if (corners[i].y < center.y)
			top.push_back(corners[i]);
		else
			bot.push_back(corners[i]);
	}

	cv::Point2f tl = top[0].x > top[1].x ? top[1] : top[0];
	cv::Point2f tr = top[0].x > top[1].x ? top[0] : top[1];
	cv::Point2f bl = bot[0].x > bot[1].x ? bot[1] : bot[0];
	cv::Point2f br = bot[0].x > bot[1].x ? bot[0] : bot[1];

	corners.clear();
	corners.push_back(tl);
	corners.push_back(tr);
	corners.push_back(bl);
	corners.push_back(br);
}*/