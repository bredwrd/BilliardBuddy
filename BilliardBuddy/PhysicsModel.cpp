#include "PhysicsModel.h"

PhysicsModel::PhysicsModel()
{
}

PhysicsModel::~PhysicsModel()
{
}

cv::vector<cv::Vec2i> PhysicsModel::calculate(cv::Mat frame, cv::vector<cv::Vec2i> pockets)
{
	//Initialize rotated matrix (image) and its size
	cv::Size rSize;
	rSize.height = int(yBot);
	rSize.width = int(xRight);
	cv::Mat rotated;

	//Initialize the coordinates of the corners after the perspective transform
	//Order is Top Left, Top Right, Bot Left, Bot Right
	cv::Point2f topLeft(xLeft, yTop);
	cv::Point2f topRight(xRight, yTop);
	cv::Point2f botLeft(xLeft, yBot);
	cv::Point2f botRight(xRight, yBot);

	//Reduce number of pockets to 4
	//Attempt to get 4 largest points on table
	if (pockets.size() > 4){
		pockets = reducePoints(frame, pockets);
	}

	//Infer an extra point if there are 3 pocket points in vector
	if (pockets.size() == 3){
		pockets = inferPoint(frame, pockets);
	}

	//Need 4 pockets to do warp perspective transform
	//For 3 points do affine transform
	if (pockets.size() == 4){
		cv::Point2f sourcePoints[4];
		cv::Point2f destPoints[4];

		for (int i = 0; i < pockets.size(); i++){
			sourcePoints[i] = cv::Point2f(float(pockets[i][0]), float(pockets[i][1]));
		}

		destPoints[0] = topLeft;
		destPoints[1] = topRight;
		destPoints[2] = botLeft;
		destPoints[3] = botRight;

		//Gets the transformation matrix
		cv::Mat warpMatrix = cv::getPerspectiveTransform(sourcePoints, destPoints);

		//Performs the warp perspective to obtain the 2D model
		cv::warpPerspective(frame, rotated, warpMatrix, rSize);

		imshow("Warped Table", rotated);
	}

	
	return ballPoints;
}

//TODO
cv::vector<cv::Vec2i> PhysicsModel::inferPoint(cv::Mat frame, cv::vector<cv::Vec2i> pockets)
{
	return pockets;
}

//TODO
cv::vector<cv::Vec2i> PhysicsModel::reducePoints(cv::Mat frame, cv::vector<cv::Vec2i> pockets)
{
	return pockets;
}