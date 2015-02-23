#include "PhysicsModel.h"

PhysicsModel::PhysicsModel()
{
}

PhysicsModel::~PhysicsModel()
{
}

cv::vector<cv::Vec2i> PhysicsModel::calculate(cv::Mat frame, cv::vector<pocket> pockets)
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

	
	return ballPoints;
}

Distances calculateDistances(cv::vector<pocket> pockets){

}

cv::vector<cv::Vec2f> calculatePath(){

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