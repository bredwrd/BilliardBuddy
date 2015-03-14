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
	if (pinkKeyPoints.size() > 3){
		pinkKeyPoints.erase(pinkKeyPoints.begin() + 3, pinkKeyPoints.end());
	}

	//Returns a vector of pocket type
	pockets = labelPockets(orangeKeyPoints, greenKeyPoints, purpleKeyPoints, pinkKeyPoints);

	return pockets;
}

cv::vector<pocket> PointLocator::labelPockets(cv::vector<cv::KeyPoint> orangeKeyPoints, cv::vector<cv::KeyPoint> greenKeyPoints,
	cv::vector<cv::KeyPoint> purpleKeyPoints, cv::vector<cv::KeyPoint> pinkKeyPoints){
	//Define vector of pocket points to be passed
	cv::vector<pocket> pockets(4);
	int pocketCount = 0;
	int realPocketCount = 0;

	//Select green pockets: Case 1: 2 green pockets in view
	if (greenKeyPoints.size() == 2){
		//Step 1: If only green pockets are seen, select destination locations based on their x values.
		if (orangeKeyPoints.size() == 0 && purpleKeyPoints.size() == 0){
			if (greenKeyPoints[0].pt.x < greenKeyPoints[1].pt.x){
				pockets[0].pocketPoints = greenKeyPoints[0];
				pockets[1].pocketPoints = greenKeyPoints[1];
			}
			else{
				pockets[0].pocketPoints = greenKeyPoints[1];
				pockets[1].pocketPoints = greenKeyPoints[0];
			}
		}
		//Step 2: If green end pockets and if both purple and orange side pockets are in view
		//Is there more logic we can use to make sure this is right? Right now it is same as just orange pockets logic.
		else if (orangeKeyPoints.size() > 0 && purpleKeyPoints.size() > 0){
			float distGreen0ToOrange = distBetweenKeyPoints(greenKeyPoints[0], orangeKeyPoints[0]);
			float distGreen1ToOrange = distBetweenKeyPoints(greenKeyPoints[1], orangeKeyPoints[0]);
			if (distGreen0ToOrange > distGreen1ToOrange){
				pockets[0].pocketPoints = greenKeyPoints[0];
				pockets[1].pocketPoints = greenKeyPoints[1];
			}
			else{
				pockets[0].pocketPoints = greenKeyPoints[1];
				pockets[1].pocketPoints = greenKeyPoints[0];
			}
		}
		//Step 3: If green end pockets and if only the orange side pocket is in view
		else if (orangeKeyPoints.size() > 0){
			float distGreen0ToOrange = distBetweenKeyPoints(greenKeyPoints[0], orangeKeyPoints[0]);
			float distGreen1ToOrange = distBetweenKeyPoints(greenKeyPoints[1], orangeKeyPoints[0]);
			if (distGreen0ToOrange > distGreen1ToOrange){
				pockets[0].pocketPoints = greenKeyPoints[0];
				pockets[1].pocketPoints = greenKeyPoints[1];
			}
			else{
				pockets[0].pocketPoints = greenKeyPoints[1];
				pockets[1].pocketPoints = greenKeyPoints[0];
			}
		}
		//Step 4: If green end pockets and if only the purple side pocket is in view
		else if (purpleKeyPoints.size() > 0){
			float distGreen0ToPurple = distBetweenKeyPoints(greenKeyPoints[0], purpleKeyPoints[0]);
			float distGreen1ToPurple = distBetweenKeyPoints(greenKeyPoints[1], purpleKeyPoints[0]);
			if (distGreen0ToPurple < distGreen1ToPurple){
				pockets[0].pocketPoints = greenKeyPoints[0];
				pockets[1].pocketPoints = greenKeyPoints[1];
			}
			else{
				pockets[0].pocketPoints = greenKeyPoints[1];
				pockets[1].pocketPoints = greenKeyPoints[0];
			}
		}

		//Removes pink keypoint candidate which is between green pockets. (Co-linear)
		removePinkCandidate(pinkKeyPoints, pockets[0], pockets[1]);

		//Puts the pockets destination locations in since top left pocket will always be pockets[0]
		pockets[0].xLocation = xLeft;
		pockets[0].yLocation = yTop;
		pockets[1].xLocation = xRight;
		pockets[1].yLocation = yTop;

		//Updates Pocket count
		pocketCount = 2;
		realPocketCount = 2;
	}

	//Step 5: Select green pockets: Case 2: 1 green pocket in view
	if (greenKeyPoints.size() == 1){
		pockets[0].pocketPoints = greenKeyPoints[0];
		if (orangeKeyPoints.size() > 0 && purpleKeyPoints.size() > 0){
			float distToOrange = distBetweenKeyPoints(greenKeyPoints[0], orangeKeyPoints[0]);
			float distToPurple = distBetweenKeyPoints(greenKeyPoints[0], purpleKeyPoints[0]);
			if (distToOrange < distToPurple){
				pockets[0].xLocation = xRight;
				pockets[0].yLocation = yTop;
			}
			else{
				pockets[0].xLocation = xLeft;
				pockets[0].yLocation = yTop;
			}
		}
		else if (orangeKeyPoints.size() > 0){
			pockets[0].xLocation = xRight;
			pockets[0].yLocation = yTop;
		}
		else if (purpleKeyPoints.size() > 0){
			pockets[0].xLocation = xLeft;
			pockets[0].yLocation = yTop;
		}
		//Updates Pocket count
		pocketCount = 1;
		realPocketCount = 1;
	}

	//Update orange and purple pockets after green pockets are in so we know that green pockets are first in vector.
	if (orangeKeyPoints.size() > 0){
		pockets[pocketCount].pocketPoints = orangeKeyPoints[0];
		pockets[pocketCount].xLocation = xRight;
		pockets[pocketCount].yLocation = yMid;
		pocketCount++;
		realPocketCount++;
	}
	if (purpleKeyPoints.size() > 0){
		pockets[pocketCount].pocketPoints = purpleKeyPoints[0];
		pockets[pocketCount].xLocation = xLeft;
		pockets[pocketCount].yLocation = yMid;
		pocketCount++;
		realPocketCount++;
	}

	//Removes pink candidates between green and orange and pink pockets
	//TODO
	if (greenKeyPoints.size() == 2){
		if (orangeKeyPoints.size() > 0){
		}
		if (purpleKeyPoints.size() > 0){
		}
	}
	else if (greenKeyPoints.size() == 1){
		if (orangeKeyPoints.size() > 0){
		}
		if (purpleKeyPoints.size() > 0){
		}
	}
	
	//Use the pink marker furthest to the left
	if (pocketCount == 2 || pocketCount == 3){
		//Determine which pink side marker is being used.
		//Should be marker closest along line between first two pockets.
		float distance = -1;
		int min = 0;
		cv::Vec2f line = lineEqn(pockets[0].pocketPoints.pt.x, pockets[0].pocketPoints.pt.y, pockets[1].pocketPoints.pt.x, pockets[1].pocketPoints.pt.y);
		for (int i = 0; i < pinkKeyPoints.size(); i++){
			float newDistance = pinkKeyPoints[i].pt.x;
			if ((distance + 1) < epsilon || newDistance < distance){
				distance = newDistance;
				min = i;
			}
		}
		if (!pinkKeyPoints.empty()){
			pockets[pocketCount].pocketPoints = pinkKeyPoints[min];
			pockets[pocketCount].xLocation = xLeft;
			pockets[pocketCount].yLocation = yMidTop;
			pocketCount++;
		}
	}

	//If 2 or 3 pockets are picked up, use any pink side marker
	/*if (pocketCount == 2 || pocketCount == 3){
		//Determine which pink side marker is being used.
		//Should be marker closest along line between first two pockets.
		float distance = -1;
		int min = 0;
		cv::Vec2f line = lineEqn(pockets[0].pocketPoints.pt.x, pockets[0].pocketPoints.pt.y, pockets[1].pocketPoints.pt.x, pockets[1].pocketPoints.pt.y);
		for (int i = 0; i < pinkKeyPoints.size(); i++){
			float newDistance = distPointToLine(pinkKeyPoints[i].pt.x, pinkKeyPoints[i].pt.y, line);
			if ((distance + 1) < epsilon || newDistance < distance){
				distance = newDistance;
				min = i;
			}
		}
		pockets[pocketCount].pocketPoints = pinkKeyPoints[min];
		pockets[pocketCount].xLocation = (pockets[0].xLocation + pockets[1].xLocation) / 2;
		pockets[pocketCount].yLocation = (pockets[0].yLocation + pockets[1].yLocation) / 2;
		pocketCount++;
	}*/

	//If 2 pockets are picked up, use a pink marker not linearly dependent with the pockets.
	//This is accomplished by finding the pink marker furthest from the line.
	/*if (realPocketCount == 2){
		//Determine which pink side marker is being used.
		//Should be marker furthest along line between pockets.
		float distance = 0;
		int max = 0;
		cv::Vec2f line = lineEqn(pockets[0].pocketPoints.pt.x, pockets[0].pocketPoints.pt.y, pockets[1].pocketPoints.pt.x, pockets[1].pocketPoints.pt.y);
		for (int i = 0; i < pinkKeyPoints.size(); i++){
			float newDistance = distPointToLine(pinkKeyPoints[i].pt.x, pinkKeyPoints[i].pt.y, line);
			if ( newDistance > distance){
				distance = newDistance;
				max = i;
			}
			pockets[pocketCount].pocketPoints = pinkKeyPoints[max];
			//Remove pink Keypoint so it doesn't get used as 4th point in the transform.

			if (!pinkKeyPoints.empty()){
				pinkKeyPoints.erase(pinkKeyPoints.begin() + max, pinkKeyPoints.begin() + max + 1);
			}
			pocketCount++;
			//Need to determine coordinates for point in perspective transform
			addNonLinearPointLocation(pockets);
		}
	}*/

	/*//If 3 pockets are picked up, use any pink side marker
	if (pocketCount == 3){
		//Determine which pink side marker is being used.
		//Should be marker closest along line between first two pockets.
		float distance = -1;
		int min = 0;
		cv::Vec2f line = lineEqn(pockets[0].pocketPoints.pt.x, pockets[0].pocketPoints.pt.y, pockets[1].pocketPoints.pt.x, pockets[1].pocketPoints.pt.y);
		for (int i = 0; i < pinkKeyPoints.size(); i++){
			float newDistance = distPointToLine(pinkKeyPoints[i].pt.x, pinkKeyPoints[i].pt.y, line);
			if ((distance + 1) < epsilon || newDistance < distance){
				distance = newDistance;
				min = i;
			}
			pockets[pocketCount].pocketPoints = pinkKeyPoints[min];

			if (!pinkKeyPoints.empty()){
				pinkKeyPoints.erase(pinkKeyPoints.begin() + min);
			}
			//Need to determine coordinates for point in perspective transform
			//First calculate distance from both known pocket points
			float distToPocket0 = distBetweenKeyPoints(pockets[0].pocketPoints, pockets[pocketCount - 1].pocketPoints);
			float distToPocket1 = distBetweenKeyPoints(pockets[1].pocketPoints, pockets[pocketCount - 1].pocketPoints);
			
			//For the case where both pockets are top pockets, pink pocket must be directly below these.
			if (inferPurple && inferOrange){
				pockets[pocketCount].yLocation = yMidTop;
				if (distToPocket0 < distToPocket1)
					pockets[pocketCount].xLocation = xLeft;
				else
					pockets[pocketCount].xLocation = xRight;
			}

			if (!inferOrange){
				if (distToPocket0 < distToPocket1){
					pockets[pocketCount].xLocation = xMid;
					pockets[pocketCount].yLocation = yTop;
				}
				else{
					//May get here in test video on accident
					//Then logic is broken.
					pockets[pocketCount].xLocation = xRight;
					pockets[pocketCount].yLocation = yMidBot;
				}
			}

			if (!inferPurple){
				if (distToPocket0 < distToPocket1){
					pockets[pocketCount].xLocation = xMid;
					pockets[pocketCount].yLocation = yTop;
				}
				else{
					//Should never get here in test video
					pockets[pocketCount].xLocation = xRight;
					pockets[pocketCount].yLocation = yMidBot;
				}
			}

			//Increase pocket count once all locations are set.
			pocketCount++;
		}
	}*/

	/*while (pockets.size() >= 2 && pockets.size() < 4 && !pinkKeyPoints.empty()){
		if (pinkKeyPoints.size() > 0){
			int i = 0;
			float distance = -1;
			
			else{
				//Find equation for line
				for (int j = 0; j < pinkKeyPoints.size(); j++){
					float newDistance = sqrt();
					if (distance == 0 || newDistance < distance){

					}

				}
				pockets[pocketCount] = pinkKeyPoints(i);

				if (!pinkKeyPoints.empty()){
					pinkKeyPoints.erase(pinkKeyPoints.begin() + i);
				}
				pocketCount++;
			}
		}
	}*/

	/*if (pocketCount == 3){
		cv::KeyPoint tempPoint = cv::KeyPoint();
		tempPoint.pt.x = (pockets[0].pocketPoints.pt.x + pockets[1].pocketPoints.pt.x) / 2;
		tempPoint.pt.y = (pockets[0].pocketPoints.pt.y + pockets[1].pocketPoints.pt.y) / 2;
		pockets[pocketCount].pocketPoints = tempPoint;
		pockets[pocketCount].xLocation = (pockets[0].xLocation + pockets[1].xLocation) / 2;
		pockets[pocketCount].yLocation = (pockets[0].yLocation + pockets[1].yLocation) / 2;
		pocketCount++;
	}*/

	return pockets;
}

cv::Vec2f PointLocator::lineEqn(float x1, float y1, float x2, float y2){
	float slope = (y2 - y1) / (x2 - x1);
	// y = mx + b
	// intercept b = y - mx
	float intercept = y1 - slope * x1;
	cv::Vec2f lineEqn = { slope, intercept };
	return lineEqn;
}

float PointLocator::distPointToLine(float x, float y, cv::Vec2f line){
	float a = line[0];
	float b = line[1];
	float distance = abs(a*x - y + b) / sqrt(pow(a, float(2)) + pow(a, float(2)));
	return distance;
}

float PointLocator::distBetweenKeyPoints(cv::KeyPoint point1, cv::KeyPoint point2){
	float x = abs(point1.pt.x - point2.pt.x);
	float y = abs(point1.pt.y - point2.pt.y);
	float distance = sqrt(pow(x, float(2)) + pow(y, float(2)));
	return distance;
}

//Adds perspective transform destination location to nonlinear pink marker point
void PointLocator::addNonLinearPointLocation(cv::vector<pocket> &pockets){
	//TODO
}

//Remove pink pocket from vector that is between 2 green points.
void PointLocator::removePinkCandidate(cv::vector<cv::KeyPoint> &pinkKeyPoints, pocket firstPocket, pocket secondPocket){
	//First check that there are actually pink pocket points
	if (!pinkKeyPoints.empty()){
		float distance = -1;
		int min = 0;
		cv::KeyPoint middlePoint;
		middlePoint.pt.x = (firstPocket.pocketPoints.pt.x + secondPocket.pocketPoints.pt.x) / 2;
		middlePoint.pt.y = (firstPocket.pocketPoints.pt.y + secondPocket.pocketPoints.pt.y) / 2;
		for (int i = 0; i < pinkKeyPoints.size(); i++){
			float newDistance = distBetweenKeyPoints(pinkKeyPoints[i], middlePoint);
			if ((distance + 1) < epsilon || newDistance < distance){
				distance = newDistance;
				min = i;
			}
		}
		pinkKeyPoints.erase(pinkKeyPoints.begin() + min, pinkKeyPoints.begin() + min + 1);
	}
}

//Last point to be added will be co-linear with first two pockets.
//This function determines its destination location after perspective transform.
//Need to check if it is in between them or outside of them.
void PointLocator::addLastPointLocation(cv::vector<pocket> &pockets, int pocketCount){
	//Set up maxs and mins for check
	float minX = fmin(pockets[0].pocketPoints.pt.x, pockets[1].pocketPoints.pt.x);
	float maxX = fmax(pockets[0].pocketPoints.pt.x, pockets[1].pocketPoints.pt.x);
	float minY = fmin(pockets[0].pocketPoints.pt.y, pockets[1].pocketPoints.pt.y);
	float maxY = fmax(pockets[0].pocketPoints.pt.y, pockets[1].pocketPoints.pt.y);

	//Checks if point is in between first two pockets.
	if (pockets[pocketCount - 1].pocketPoints.pt.x > minX &&
		pockets[pocketCount - 1].pocketPoints.pt.x < maxX &&
		pockets[pocketCount - 1].pocketPoints.pt.y > minY &&
		pockets[pocketCount - 1].pocketPoints.pt.y < maxY){
		pockets[pocketCount - 1].xLocation = (pockets[0].xLocation + pockets[1].xLocation) / 2;
		pockets[pocketCount - 1].yLocation = (pockets[0].yLocation + pockets[1].yLocation) / 2;
	}
	else{
		float distToPocket0 = distBetweenKeyPoints(pockets[0].pocketPoints, pockets[pocketCount - 1].pocketPoints);
		float distToPocket1 = distBetweenKeyPoints(pockets[1].pocketPoints, pockets[pocketCount - 1].pocketPoints);
		if ((pockets[0].xLocation - pockets[1].xLocation) < epsilon){
			pockets[pocketCount - 1].xLocation = pockets[0].xLocation;
			//This is hardcoded in for our test cases
			//TODO is complete logic here later if we have different coloured bottom pockets
			pockets[pocketCount - 1].yLocation = yMidBot;
		}
		else
		{
			//Should never get here
			//This is failsafe so program doesn't crash
			pockets[pocketCount - 1].yLocation = pockets[0].yLocation;
			pockets[pocketCount - 1].yLocation = yMidTop;
		}
	}
}