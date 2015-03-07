#include "PhysicsModel.h"

PhysicsModel::PhysicsModel()
{
}

PhysicsModel::~PhysicsModel()
{
}

cv::vector<Path> PhysicsModel::calculate(cv::Mat frame, cv::vector<pocket> pockets, cv::vector<cv::Vec2i> cue,
	cv::vector<cv::Vec2i> whiteBall, cv::vector<cv::Vec2i> balls)
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
	cv::vector<Path> trajectoryPoints;
	
	//Calculates pathVector (a vector filled with start and end points for the visual augmentor)
	calculateTrajectories(trajectoryPoints, whiteBall, balls, cue);

	return trajectoryPoints;
}

void PhysicsModel::calculateTrajectories(cv::vector<Path>& trajectoryPoints, cv::vector<cv::Vec2i> cueBall, cv::vector<cv::Vec2i> targetBalls, cv::vector<cv::Vec2i> cueStick){

	//Convert input coordinates to floats
	cv::vector<cv::Vec2f> cueBallf(cueBall.size());
	cv::vector<cv::Vec2f> targetBallsf(targetBalls.size());
	cv::vector<cv::Vec2f> cueStickf(cueStick.size());
	cueBallf[0][0] = (float)cueBall[0][0];
	cueBallf[0][1] = (float)cueBall[0][1];
	cueStickf[0][0] = (float)cueStick[0][0];
	cueStickf[0][1] = (float)cueStick[0][1];
	cueStickf[1][0] = (float)cueStick[1][0];
	cueStickf[1][1] = (float)cueStick[1][1];
	for (int i = 0; i < targetBalls.size(); i++){
		targetBallsf[i][0] = (float)targetBalls[i][0];
		targetBallsf[i][1] = (float)targetBalls[i][1];
	}
	
	//Check if cue stick crosses cue ball
	cv::vector<cv::Vec2f> CueBallParams = getContactPointParams(cueStickf[0],cueStickf[1],cueBallf,0,cueBallRadius);
	
	////Cue stick crosses cue ball and cue stick is close enough to cue ball
	if ((targetBalls.size() != 0) && (CueBallParams.size() != 0) && (norm(cueStickf[1],cueBallf[0]) < (float)maxCueDist)){
		trajectoryPoints = getTrajectoryGroup(CueBallParams[1], CueBallParams[4], targetBallsf, 0, maxTotalTrajs, 2*ballRadius);
	}

	return;

}

cv::vector<Path> PhysicsModel::getTrajectoryGroup(cv::Vec2f param1, cv::Vec2f param2, cv::vector<cv::Vec2f> targetBalls, int ballIndex, int trajMax, float radius)
{
	//Initialize empty trajectory Path vector
	cv::vector<Path> trajectoryPoints;

	//Calculate start and end points of trajectories
	int t = 0;
	while (t < trajMax){

		Path traj;
		traj.startPoint = param2;
		cv::vector<cv::Vec2f> ballParams = getContactPointParams(param1, param2, targetBalls, ballIndex, radius);

		if (ballParams.size() != 0){
			ballIndex = (int)ballParams[0][0];
			traj.endPoint = ballParams[1];
			param1 = ballParams[2];
			param2 = ballParams[3];
			trajectoryPoints.push_back(traj);
		}
		t++;
	}
	return trajectoryPoints;
}

cv::vector<cv::Vec2f> PhysicsModel::getContactPointParams(cv::Vec2f param1,cv::Vec2f param2,cv::vector<cv::Vec2f> targetBalls,int targetBallIndex,float radius)
{
	//Define empty vectors to store data, and determine number of target balls
	cv::vector<cv::Vec2f> nextParams(0);
	cv::vector<cv::Vec2f> ballCandidates(targetBalls.size());
	cv::vector<float> candidateDists(targetBalls.size());
	int B = targetBalls.size();

	//Find all intersections between line formed by two (x,y) coordinates and balls specified by arg "targetBalls"
	for (int b = 1; b <= B; b++){
		if (b != targetBallIndex){
			cv::vector<cv::Vec2f> candidate = backPointFromLine(param1,param2,targetBalls[b-1],radius);
			if (candidate.size() == 1){
				ballCandidates[b-1] = candidate[0];
			}
		}
	}

	//Filter out all candidates which are not properly oriented
	for (int b = 1; b <= B; b++){
		if (norm(ballCandidates[b-1],param1) < norm(ballCandidates[b-1],param2)){
			ballCandidates[b-1] = { 0, 0 };
		}
	}

	//Calculate distances from candidates to param2
	for (int b = 1; b <= B; b++){
		if ((ballCandidates[b-1][0] == 0) && (ballCandidates[b-1][1] == 0)){
			candidateDists[b-1] = 0;
		}
		else{
			candidateDists[b-1] = norm(ballCandidates[b-1],param2);
		}
	}

	//Check if sufficient candidates exist, and find its index
	int candidate_exists = 0;
	int i = 1;
	int min_index = 0;
	while ((candidate_exists == 0) && (i <= B)){
		if (candidateDists[i - 1] != 0){
			candidate_exists = 1;
			min_index = i;
		}
		i++;
	}

	//If sufficient candidates exist, find the sufficient candidate.
	if (candidate_exists == 1){
		float min_dist = candidateDists[min_index-1];
		for (int i = 1; i <= B;i++){
			if ((candidateDists[i-1] < min_dist)&&(candidateDists[i-1] != 0)){
				min_index = i;
			}
		}
	}	

	//If sufficient candidate exists, return its parameters, otherwise return nothing
	if (candidate_exists == 1){
		//store index of hit ball
		cv::Vec2f nextTargetBallIndex = { (float)min_index , 0 };
		nextParams.push_back(nextTargetBallIndex);
		//store 2r back point of hit ball
		cv::Vec2f backPoint2r = ballCandidates[min_index - 1];
		nextParams.push_back(backPoint2r);
		// store r back point of hit ball
		cv::Vec2f backPointr = backFromBack(backPoint2r, targetBalls[min_index - 1]);
		nextParams.push_back(backPointr);
		// store r front point of hit ball
		cv::Vec2f frontPointr = frontFromBack(backPointr, targetBalls[min_index - 1]);
		nextParams.push_back(frontPointr);
		// store 2r front point of hit ball
		cv::Vec2f frontPoint2r = frontFromBack(backPoint2r, targetBalls[min_index - 1]);
		nextParams.push_back(frontPoint2r);
	}

	return nextParams;
}

cv::vector<cv::Vec2f> PhysicsModel::backPointFromLine(cv::Vec2f param1, cv::Vec2f param2, cv::Vec2f target, float radius){

	//Calculate parameters of a line formed by param1 and param2, and shorten circle parameters for convenience
	float m;
	float b;
	float c;
	float d;
	int invert_flag;
	if (param2[0] == param1[0]){
		m = 0;
		b = param1[0];
		c = target[1];
		d = target[0];
		invert_flag = 1;
	}
	else{
		m = (param2[1] - param1[1]) / (param2[0] - param1[0]);
		b = param1[1] - m*param1[0];
		c = target[0];
		d = target[1];
	}

	//Find discriminant to check if intersection exists
	float discrim = pow((2*(m*(b-d)-c)),2)-4*(pow(m,2)+1)*(pow(b,2)+pow(c,2)+pow(d,2)-2*b*d-pow(radius,2));

	//Initialize solution variables
	float x_sol_1;
	float x_sol_2;
	cv::Vec2f sol_1;
	cv::Vec2f sol_2;
	cv::vector<cv::Vec2f> backPoint;

	//Determine solutions, if they exist. In the case of two solutions, find solution closer to param2
	if(discrim == 0){
		x_sol_1 = (-2 * (m*(b - d) - c)) / (2 * (pow(m, 2) + 1));
		sol_1 = { x_sol_1 , m*x_sol_1 + b };
		backPoint.push_back(sol_1);
	}
	else if(discrim > 0){
		x_sol_1 = ((-2 * (m*(b - d) - c)) + sqrt(discrim)) / (2 * (pow(m, 2) + 1));
		x_sol_2 = ((-2 * (m*(b - d) - c)) - sqrt(discrim)) / (2 * (pow(m, 2) + 1));
		sol_1 = { x_sol_1, m*x_sol_1 + b };
		sol_2 = { x_sol_2, m*x_sol_2 + b };
		if (norm(sol_1,param2) < norm(sol_2,param2)){
			backPoint.push_back(sol_1);
		}
		else{
			backPoint.push_back(sol_2);
		}
	}

	//Swap values if original problem was inverted
	if ((invert_flag == 1)&&(backPoint.size() == 1)){
		float placeholder = backPoint[0][0];
		backPoint[0][0] = backPoint[0][1];
		backPoint[0][1] = placeholder;
	}
	
	return backPoint;

}

cv::Vec2f PhysicsModel::frontFromBack(cv::Vec2f backPoint, cv::Vec2f ballCoord){
	
	float DeltaX = ballCoord[0] - backPoint[0];
	float DeltaY = ballCoord[1] - backPoint[1];
	cv::Vec2f Delta = { DeltaX, DeltaY };
	cv::Vec2f newFrontPoint = ballCoord + Delta;
	return newFrontPoint;

}

cv::Vec2f PhysicsModel::backFromBack(cv::Vec2f backPoint, cv::Vec2f ballCoord){
	
	float DeltaX = ballCoord[0] - backPoint[0];
	float DeltaY = ballCoord[1] - backPoint[1];
	cv::Vec2f Delta = { DeltaX, DeltaY };
	cv::Vec2f newBackPoint = ballCoord - 0.5*Delta;
	return newBackPoint;

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