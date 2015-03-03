#include "PointLocator.h"

PointLocator::PointLocator()
{
}

PointLocator::~PointLocator()
{
}

cv::vector<pocket> PointLocator::infer(cv::vector<cv::KeyPoint> orangeKeyPoints, cv::vector<cv::KeyPoint> greenKeyPoints,
										cv::vector<cv::KeyPoint> purpleKeyPoints, cv::vector<cv::KeyPoint> pinkKeyPoints)
{
	//Define vector of pocket points to be passed
	cv::vector<pocket> pockets;

	//There should be a maximum of 2 points per colour. If there is more, reduce.
	//This depends on the quality of test video results.
	//Right now it just takes the first 2 points in vector to prevent crashes.
	//Takes only one point for orange and purple since they are side pockets
	//TODO if needed later.
	if (orangeKeyPoints.size() > 1){
		orangeKeyPoints.erase(orangeKeyPoints.begin() + 1, orangeKeyPoints.end());
	}
	if (greenKeyPoints.size() > 2){
		greenKeyPoints.erase(greenKeyPoints.begin() + 2, greenKeyPoints.end());
	}
	if (purpleKeyPoints.size() > 1){
		purpleKeyPoints.erase(purpleKeyPoints.begin() + 1, purpleKeyPoints.end());
	}
	if (pinkKeyPoints.size() > 2){
		pinkKeyPoints.erase(pinkKeyPoints.begin() + 2, pinkKeyPoints.end());
	}

	/*int pocketCount = orangeKeyPoints.size() + greenKeyPoints.size() + purpleKeyPoints.size() + pinkKeyPoints.size();
	//Create vector with all pocket points after reduction (Size should be 3-6)
	cv::vector<cv::KeyPoint> allPockets(pocketCount);
	for (int i = 0; i < orangeKeyPoints.size(); i++){
		allPockets.push_back(orangeKeyPoints[i]);
	}
	for (int i = 0; i < greenKeyPoints.size(); i++){
		allPockets.push_back(greenKeyPoints[i]);
	}
	for (int i = 0; i < purpleKeyPoints.size(); i++){
		allPockets.push_back(purpleKeyPoints[i]);
	}
	for (int i = 0; i < pinkKeyPoints.size(); i++){
		allPockets.push_back(pinkKeyPoints[i]);
	}*/

	//Returns a vector of pocket type
	pockets = labelPockets(orangeKeyPoints, greenKeyPoints, purpleKeyPoints, pinkKeyPoints);

	return pockets;
}

cv::vector<pocket> PointLocator::labelPockets(cv::vector<cv::KeyPoint> orangeKeyPoints, cv::vector<cv::KeyPoint> greenKeyPoints,
	cv::vector<cv::KeyPoint> purpleKeyPoints, cv::vector<cv::KeyPoint> pinkKeyPoints){
	//Define vector of pocket points to be passed
	cv::vector<pocket> pockets(4);
	int pocketCount = 0;
	bool inferOrange = true;
	bool inferPurple = true;

	if (greenKeyPoints.size() == 2){
		if (greenKeyPoints[0].pt.x < greenKeyPoints[1].pt.x){
			pockets[0].pocketPoints = greenKeyPoints[0];
			pockets[1].pocketPoints = greenKeyPoints[1];
		}
		else{
			pockets[0].pocketPoints = greenKeyPoints[1];
			pockets[1].pocketPoints = greenKeyPoints[0];
		}
		pockets[0].xLocation = xLeft;
		pockets[0].yLocation = yTop;
		pockets[1].xLocation = xRight;
		pockets[1].yLocation = yTop;

		//Updates Pocket count
		pocketCount += 2;
	}
	if (orangeKeyPoints.size() > 0){
		pockets[pocketCount].pocketPoints = orangeKeyPoints[0];
		pockets[pocketCount].xLocation = xLeft;
		pockets[pocketCount].yLocation = yBot;
		pocketCount++;
		bool inferOrange = false;
	}
	if (purpleKeyPoints.size() > 0){
		pockets[pocketCount].pocketPoints = purpleKeyPoints[0];
		pockets[pocketCount].xLocation = xRight;
		pockets[pocketCount].yLocation = yBot;
		pocketCount++;
		bool inferPurple = false;
	}

	//Infer 4th point Simple Case of 3 Pockets (2 Green & 1 Purple or Orange)
	/*if (pockets.size() == 3){
		cv::KeyPoint tempPoint = cv::KeyPoint();
		if (inferOrange){
			tempPoint.pt.x = (pockets[2].xLocation + pockets[1].xLocation) / 2;
			tempPoint.pt.y = (pockets[2].yLocation + pockets[1].yLocation) / 2;
		}
		if (inferPurple){
			tempPoint.pt.x = (pockets[2].xLocation + pockets[0].xLocation) / 2;
			tempPoint.pt.y = (pockets[2].yLocation + pockets[0].yLocation) / 2;
		}
	}*/
	if (pockets.size() == 3){
		cv::KeyPoint tempPoint = cv::KeyPoint();
		tempPoint.pt.x = (pockets[0].xLocation + pockets[1].xLocation) / 2;
		tempPoint.pt.y = (pockets[0].yLocation + pockets[1].yLocation) / 2;
		pockets[pocketCount].pocketPoints = tempPoint;
		pockets[pocketCount].xLocation = xMid;
		pockets[pocketCount].yLocation = yTop;
		pocketCount++;
	}

	return pockets;
}