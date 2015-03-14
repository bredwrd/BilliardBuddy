#include "PhysicsModel.h"
#include "TrajectoryAugmentor.h"
#include <cmath>
#include <iostream>
using namespace std;

PhysicsModel::PhysicsModel()
{
}

PhysicsModel::~PhysicsModel()
{
}

cv::vector<Path> PhysicsModel::calculate(cv::Mat frame, cv::vector<pocket> pockets, cv::vector<cv::Vec2i> cueStick,
	cv::vector<cv::Vec2f> cueBall, cv::vector<cv::Vec2f> targetBalls)
{

	//Initialize path vector to be returned (trajectory points in 3D space)
	cv::vector<Path> trajectoryPoints3D;
	
	//Need 4 pockets to do warp perspective transform
	if (pockets.size() == 4){
		cv::vector<cv::Point2f> sourcePoints(4);
		cv::vector<cv::Point2f> destPoints(4);

		//Form four corners in both the source and destination spaces
		for (int i = 0; i < 4; i++){
			sourcePoints[i] = cv::Point2f(pockets[i].pocketPoints.pt.x, pockets[i].pocketPoints.pt.y);
			destPoints[i] = cv::Point2f(pockets[i].xLocation, pockets[i].yLocation);
		}
		
		//// Get mass center
		//cv::Point2f center(0, 0);
		//for (int i = 0; i < sourcePoints.size(); i++)
		//	center += sourcePoints[i];

		//center *= (1. / sourcePoints.size());
		//sortCorners(sourcePoints, center);

		//Compute transformation matrix for 3D --> 2D and 2D --> 3D respectively
		cv::Mat forwardMatrix = cv::getPerspectiveTransform(sourcePoints, destPoints);
		cv::Mat backwardMatrix = cv::getPerspectiveTransform(destPoints, sourcePoints);

		///Convert input coordinates for cue stick into floats //TEMP
		cv::vector<cv::Vec2f> cueStickf(cueStick.size());
		cueStickf[0][0] = (float)cueStick[0][0];
		cueStickf[0][1] = (float)cueStick[0][1];
		cueStickf[1][0] = (float)cueStick[1][0];
		cueStickf[1][1] = (float)cueStick[1][1];

		// Input: 3D coordinates as Vec2f. Output: 2D coordinates as Point2f
		cv::vector<cv::Point2f> cueBall2D(cueBall.size());
		cv::vector<cv::Point2f> targetBalls2D(targetBalls.size());
		cv::vector<cv::Point2f> cueStickf2D(cueStick.size());
		cv::vector<cv::Point2f> cueBall3D(cueBall.size());
		cv::vector<cv::Point2f> targetBalls3D(targetBalls.size());
		cv::vector<cv::Point2f> cueStickf3D(cueStick.size());
		cueBall3D[0] = cv::Point2f(cueBall[0][0],cueBall[0][1]);
		cueStickf3D[0] = cv::Point2f(cueStickf[0][0],cueStickf[0][1]);
		cueStickf3D[1] = cv::Point2f(cueStickf[1][0], cueStickf[1][1]);
		for (int b = 0; b < targetBalls.size(); b++){
			targetBalls3D[b] = cv::Point2f(targetBalls[b][0],targetBalls[b][1]);
		}
		cv::perspectiveTransform(cueBall3D, cueBall2D, forwardMatrix);
		cv::perspectiveTransform(targetBalls3D, targetBalls2D, forwardMatrix);
		cv::perspectiveTransform(cueStickf3D, cueStickf2D, forwardMatrix);

		//Input: 2D coordinates as Point2f. Output: 2D coordinates as Vec2f, y coord adjusted for physics, cue stick back and front point switched
		cv::vector<cv::Vec2f> cueBall2DVec(cueBall.size());
		cv::vector<cv::Vec2f> targetBalls2DVec(targetBalls.size());
		cv::vector<cv::Vec2f> cueStickf2DVec(cueStickf.size());
		cueBall2DVec[0] = {cueBall2D[0].x,360 - cueBall2D[0].y};
		cueStickf2DVec[1] = {cueStickf2D[0].x,360 - cueStickf2D[0].y}; // temp reverse back and front point, so 0th is back, and 1th is front
		cueStickf2DVec[0] = { cueStickf2D[1].x,360 - cueStickf2D[1].y };
		for (int b = 0; b < targetBalls.size(); b++){
			targetBalls2DVec[b] = {targetBalls2D[b].x,360 - targetBalls2D[b].y};
		}

		//temp wizard of OZ cue ball and target ball coords
		cueBall2DVec[0] = { 74.25, 108 }; //temp
		targetBalls2DVec[0] = { 211.5, 225 }; //temp

		//readjust cue stick to hit center of cue ball
		float cueDX = (cueStickf2DVec[0][0] - cueStickf2DVec[1][0]);
		float cueDY = (cueStickf2DVec[0][1] - cueStickf2DVec[1][1]);
		float cueSlope = (cueDY/cueDX);
		float cueLength = (norm(cueStickf2DVec[0],cueStickf2DVec[1]));
		//float cueLength = norm(cueStickf2DVec[0],cueStickf2DVec[1]);
		cv::vector<cv::Vec2f> newCueFrontPoint(1);
		newCueFrontPoint[0][0] = cueBall2DVec[0][0] - 2*ballRadius*cos(atan(abs(cueDY)/abs(cueDX)));
		newCueFrontPoint[0][1] = cueBall2DVec[0][1] - 2 * ballRadius*sin(atan(abs(cueDY) / abs(cueDX)));
		cv::vector<cv::Vec2f> newCueBackPoint(1);
		newCueBackPoint[0][0] = cueBall2DVec[0][0] - cueLength*cos(atan(abs(cueDY) / abs(cueDX)));
		newCueBackPoint[0][1] = cueBall2DVec[0][1] - cueLength*sin(atan(abs(cueDY) / abs(cueDX)));
		cv::vector<cv::Vec2f> newCuePoint;
		newCuePoint.push_back(newCueBackPoint[0]);
		newCuePoint.push_back(newCueFrontPoint[0]);

		//Do the physics calculations
		cv::vector<Path> trajectoryPoints2D;
		calculateTrajectories(trajectoryPoints2D, cueBall2DVec, targetBalls2DVec, newCuePoint);

		//Input: 2D trajectories in coordinates of physics calculations. Output: 2D trajectories in coordinates ready to draw in a frame
		for (int k = 0; k < trajectoryPoints2D.size(); k++){
			cv::Vec2f tempvec1 = trajectoryPoints2D[k].startPoint;
			cv::Vec2f tempvec2 = trajectoryPoints2D[k].endPoint;
			tempvec1[1] = 360 - tempvec1[1];
			tempvec2[1] = 360 - tempvec2[1];
			trajectoryPoints2D[k].startPoint = tempvec1;
			trajectoryPoints2D[k].endPoint = tempvec2;
		}
		
		//Shows 2D perspective transformed video with 2D trajectories
		cv::Mat rotated;
		cv::Size rSize = { 360, 360 };
		cv::warpPerspective(frame, rotated, forwardMatrix, rSize);
		TrajectoryAugmentor trajectoryAugmentor = TrajectoryAugmentor();
		trajectoryAugmentor.augment(rotated, trajectoryPoints2D);
		imshow("4 Point Warped Table", rotated);

		//for (int i = 0; i < trajectoryPoints2D.size(); i++){
		//	cout << "trajectory points 2D " << i << endl;
		//	cout << trajectoryPoints2D[i].startPoint << endl;
		//	cout << trajectoryPoints2D[i].endPoint << endl;
		//}

		//Convert output 2D trajectory coordinates into points
		cv::vector<cv::Point2f> trajectoryPoints2D_StartPoints(trajectoryPoints2D.size());
		cv::vector<cv::Point2f> trajectoryPoints2D_EndPoints(trajectoryPoints2D.size());
		for (int t = 0; t < trajectoryPoints2D.size(); t++){
			cv::Vec2f tempStart = trajectoryPoints2D[t].startPoint;
			cv::Vec2f tempEnd = trajectoryPoints2D[t].endPoint;
			trajectoryPoints2D_StartPoints[t] = cv::Point2f(tempStart[0], tempStart[1]);
			trajectoryPoints2D_EndPoints[t] = cv::Point2f(tempEnd[0], tempEnd[1]);
		}

		//Transform the 2D trajectory coordinates back into the 3D space
		cv::vector<cv::Point2f> trajectoryPoints3D_StartPoints(trajectoryPoints2D_StartPoints.size());
		cv::vector<cv::Point2f> trajectoryPoints3D_EndPoints(trajectoryPoints2D_EndPoints.size());
		if (trajectoryPoints2D_StartPoints.size()!=0){
			cv::perspectiveTransform(trajectoryPoints2D_StartPoints, trajectoryPoints3D_StartPoints, backwardMatrix);
			cv::perspectiveTransform(trajectoryPoints2D_EndPoints, trajectoryPoints3D_EndPoints, backwardMatrix);

			//Convert output 3D trajectory coordinates into Path
			//cv::vector<Path> trajectoryPoints3D(trajectoryPoints3D.size());
			for (int i = 0; i < trajectoryPoints3D_StartPoints.size(); i++){
				trajectoryPoints3D.push_back({ 0, 0 });
				trajectoryPoints3D[i].startPoint = { trajectoryPoints3D_StartPoints[i].x, trajectoryPoints3D_StartPoints[i].y };
				trajectoryPoints3D[i].endPoint = { trajectoryPoints3D_EndPoints[i].x, trajectoryPoints3D_EndPoints[i].y };
			}

		}
	
	}

	return trajectoryPoints3D;

}

void PhysicsModel::calculateTrajectories(cv::vector<Path>& trajectoryPoints, cv::vector<cv::Vec2f> cueBall, cv::vector<cv::Vec2f> targetBalls, cv::vector<cv::Vec2f> cueStick){
	
	//Check if cue stick crosses cue ball
	cv::vector<cv::Vec2f> CueBallParams = getContactPointParams(0,cueStick[0],cueStick[1],cueBall,0,cueBallRadius);

	//Cue stick crosses cue ball and cue stick is close enough to cue ball
	if ((CueBallParams[0][0] == 0) && (norm(cueStick[1],cueBall[0]) < (float)maxCueDist)){
		trajectoryPoints = getTrajectoryGroup(CueBallParams[1], CueBallParams[4], targetBalls, 0);
	}

	return;

}

cv::vector<Path> PhysicsModel::getTrajectoryGroup(cv::Vec2f param1, cv::Vec2f param2, cv::vector<cv::Vec2f> targetBalls, int ballIndex)
{
	//Initialize variables
	Path mainTrajectory;
	Path branchTrajectory;
	cv::vector<Path> trajectoryPoints;
	cv::vector<cv::Vec2f> mainResults;
	cv::vector<cv::Vec2f> branchResults;
	cv::Vec2f ghostSlope;
	cv::vector<cv::Vec2f> ghostPoint1(1);
	cv::vector<cv::Vec2f> ghostPoint2(1);
	cv::vector<cv::Vec2f> ghostBackPoint(1);
	cv::vector<cv::Vec2f> ghostFrontPoint(1);
	int edgeCount = 0;
	int t = 1;
	//float maxBranchLength = cutFactor*(abs(pockets[0].y-pockets[3].y));
	int featureIndex = 0;
	int branchFlag;

	//Calculate start and end points of trajectories
	while ((t <= maxTotalTrajs)&&(edgeCount <= maxEdgeTrajs)){

		//Set start point
		if (featureIndex == 0){
			mainTrajectory.startPoint = param2;
		}
		else{
			mainTrajectory.startPoint = param1;
		}

		//Obtain parameters on next contacted feature in main trajectory group
		mainResults = getContactPointParams(featureIndex, param1, param2, targetBalls, ballIndex, 2*ballRadius);
		mainTrajectory.endPoint = mainResults[1];
		trajectoryPoints.push_back(mainTrajectory);

		//Establish param1 and param2 for the next loop. If ball was hit, calculate branch trajectories as well.
		if (mainResults[0][0] == 0){ //ball was hit
			
			//Calculate branch points
			branchTrajectory.startPoint = mainResults[1];
			if (mainResults[2][0] == mainResults[3][0]){ //infinity slope case
				ghostPoint1[0][0] = mainResults[1][0];
				ghostPoint1[0][1] = mainResults[1][1]-ballRadius;
				ghostPoint2[0][0] = mainResults[1][0];
				ghostPoint2[0][1] = mainResults[1][1]+ballRadius;
			}
			else{
				ghostSlope = -1/((mainResults[3][1]-mainResults[2][1])/(mainResults[3][0]-mainResults[2][0]));
				ghostPoint1 = backPointFromLine(1,mainResults[1],ghostSlope,mainResults[1],ballRadius);
				ghostPoint2[0] = frontFromBack(ghostPoint1[0],mainResults[1]);
			}
			if (norm(ghostPoint1[0], param1) < norm(ghostPoint2[0], param1)){
				ghostFrontPoint = ghostPoint2;
				ghostBackPoint = ghostPoint1;
				branchFlag = 1;
			}
			else if (norm(ghostPoint1[0],param1) > norm(ghostPoint2[0],param1)){
				ghostFrontPoint = ghostPoint1;
				ghostBackPoint = ghostPoint2;
				branchFlag = 1;
			}
			else{
				branchFlag = 0;
			}

			if (branchFlag == 1){
				branchResults = getContactPointParams(0,ghostBackPoint[0],ghostFrontPoint[0],targetBalls,ballIndex,2*ballRadius);
				branchTrajectory.endPoint = branchResults[1];
				trajectoryPoints.push_back(branchTrajectory);
			}
			
			//Next loop info for main trajectory
			featureIndex = 0;
			param1 = mainResults[2];
			param2 = mainResults[3];
			ballIndex = (int)mainResults[5][0];
			//edgeCount = 0 //not sure if will be useful

		}
		else if (mainResults[0][0] == 1){ //edge was hit
			featureIndex = 1;
			param1 = mainResults[1];
			param2 = mainResults[2];
			edgeCount++;
		}
		else { //pocket was hit
			t = maxTotalTrajs;
		}
		t++;
	}
	return trajectoryPoints;
}

cv::vector<cv::Vec2f> PhysicsModel::getContactPointParams(int featureIndex,cv::Vec2f param1,cv::Vec2f param2,cv::vector<cv::Vec2f> targetBalls,int targetBallIndex,float radiusBall)
{
	//Define empty vectors to store data, and determine number of target balls
	cv::vector<cv::Vec2f> nextParams;
	int B = targetBalls.size();
	cv::vector<cv::Vec2f> ballCandidates(B);
	cv::vector<float> candidateDists(B);
	int ballCandidateExists = 0; // assume there are no intersected balls prior to finding one

	//Find all intersections between line and target balls not including previously struck ball
	for (int b = 1; b <= B; b++){
		if (b != targetBallIndex){
			cv::vector<cv::Vec2f> candidate = backPointFromLine(featureIndex,param1,param2,targetBalls[b-1],radiusBall);
			if (candidate.size() == 1){
				ballCandidates[b-1] = candidate[0];
			}
		}
	}

	//Calculate distances from candidates to param1 (works both for input being ball or edge)
	for (int b = 1; b <= B; b++){
		if ((ballCandidates[b-1][0] == 0) && (ballCandidates[b-1][1] == 0)){
			candidateDists[b-1] = 0;
		}
		else{
			candidateDists[b-1] = norm(ballCandidates[b-1],param1);
		}
	}

	//Check if sufficient candidates exist, and find its index
	int i = 1;
	int ballMinIndex = 0;
	while ((ballCandidateExists == 0) && (i <= B)){
		if (candidateDists[i - 1] != 0){
			ballCandidateExists = 1;
			ballMinIndex = i;
		}
		i++;
	}

	//If sufficient ball candidates exist, find the sufficient one, and determine output parameters. 
	//Otherwise, determine output parameters for edge or pocket, if they are hit.
	if (ballCandidateExists == 1){
		float minDist = candidateDists[ballMinIndex-1];
		for (int i = 1; i <= B;i++){
			if ((candidateDists[i-1] < minDist)&&(candidateDists[i-1] != 0)){
				ballMinIndex = i;
			}
		}
		nextParams.push_back({ 0, 0 });
		cv::Vec2f backPoint2R = ballCandidates[ballMinIndex-1];
		nextParams.push_back(backPoint2R);
		cv::Vec2f backPointR = backFromBack(backPoint2R,targetBalls[ballMinIndex-1]);
		nextParams.push_back(backPointR);
		cv::Vec2f frontPointR = frontFromBack(backPointR,targetBalls[ballMinIndex-1]);
		nextParams.push_back(frontPointR);
		cv::Vec2f frontPoint2R = frontFromBack(backPoint2R,targetBalls[ballMinIndex-1]);
		nextParams.push_back(frontPoint2R);
		nextParams.push_back({(float)ballMinIndex,0});
	}	
	else{
		cv::vector<cv::Vec2f> pocketIntPoint = pocketPointFromLine(featureIndex,param1,param2,pocketRadius);
		if (pocketIntPoint.size()==0){ //edge hit
			nextParams.push_back({ 1, 0 });
			cv::vector<cv::Vec2f> edgeIntPoint = edgePointFromLine(featureIndex,param1,param2);
			nextParams.push_back(edgeIntPoint[0]);
			cv::Vec2f reflectSlope;
			if (featureIndex == 0){ //input was ball
				if (param1[0]==param2[0]){ //infinity slope case
					reflectSlope = { 0, 1 };
				}
				else{
					reflectSlope = { (-(param2[1]-param1[1])/(param2[0]-param1[0])), 0 };
				}
			}
			else{ //input was edge
				if (param2[1]==1){ //infinity slope case
					reflectSlope = { 0, 1 };
				}
				else{
					reflectSlope = { -param2[0], 0 };
				}
			}
			nextParams.push_back(reflectSlope);
		}
		else{ //pocket hit
			nextParams.push_back({ 2, 0 });
			nextParams.push_back(pocketIntPoint[0]);
		}
	}

	return nextParams;

}

cv::vector<cv::Vec2f> PhysicsModel::backPointFromLine(int featureIndex, cv::Vec2f param1, cv::Vec2f param2, cv::Vec2f target, float radius){

	//Calculate parameters of a line formed by param1 and param2, depending on input feature. Shorten variables for convenience
	float m;
	float b;
	float c;
	float d;
	int invert_flag = 0;

	if (featureIndex == 0){ //two x,y coords as input
		if (param1[0] == param2[0]){ //if slope of line is infinite, solve the inverted problem
			m = 0;
			b = param1[0];
			c = target[1];
			d = target[0];
			invert_flag = 1;
		}
		else{ //solve normal (non-inverted) problem
			m = (param2[1] - param1[1]) / (param2[0] - param1[0]);
			b = param1[1] - m*param1[0];
			c = target[0];
			d = target[1];
		}
	}
	else{ //x,y coord and slope as input
		if (param2[1]==1){ //if slope of line is infinite, solve the inverted problem
			m = 0;
			b = param1[0];
			c = target[1];
			d = target[0];
			invert_flag = 1;
		}
		else{ //solve normal (non-inverted) problem
			m = param2[0];
			b = param1[1] - m*param1[0];
			c = target[0];
			d = target[1];
		}
	}

	//Find discriminant to check if intersection exists
	float discrim = pow((2*(m*(b-d)-c)),2)-4*(pow(m,2)+1)*(pow(b,2)+pow(c,2)+pow(d,2)-2*b*d-pow(radius,2));

	//Initialize solution variables
	float x_sol_1;
	float x_sol_2;
	cv::Vec2f sol_1;
	cv::Vec2f sol_2;
	cv::vector<cv::Vec2f> backPoint;

	//Determine solutions, if they exist. In the case of two solutions, find solution closer to input feature
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
		if (norm(sol_1,param1) < norm(sol_2,param1)){
			backPoint.push_back(sol_1);
		}
		else{
			backPoint.push_back(sol_2);
		}
		//if input feature is two x,y coords, eliminate back point if it is not properly oriented
		if ((featureIndex == 0)&&(norm(backPoint[0],param1)<norm(backPoint[0],param2))){
			backPoint.clear();
		}
	}

	//Swap values if original problem was inverted
	if ((invert_flag == 1)&&(backPoint.size() == 1)){
		float placeHolder = backPoint[0][0];
		backPoint[0][0] = backPoint[0][1];
		backPoint[0][1] = placeHolder;
	}
	
	return backPoint;

}

cv::vector<cv::Vec2f> PhysicsModel::edgePointFromLine(int featureIndex, cv::Vec2f param1, cv::Vec2f param2){
	
	cv::vector<cv::Vec2f> edgeIntPoint;
	cv::vector<cv::Vec2f> edgePoint1;
	cv::vector<cv::Vec2f> edgePoint2;
	float slope;
	float yint;

	if (featureIndex == 0){ //input param is ball
		if (param1[0]==param2[0]){ //infinity slope case
			edgePoint1.push_back({param1[0],PT_UL[1]});
			edgePoint2.push_back({param1[0],PT_BL[1]});
		}
		else if (param1[1]==param2[1]){
			edgePoint1.push_back({PT_BL[0],param1[1]});
			edgePoint2.push_back({PT_BR[0],param1[1]});
		}
		else{
			slope = (param1[1]-param2[1]) / (param1[0]-param2[0]);
			yint = param1[1] - slope*param1[0];
		}
	}
	else{ //input param is edge
		if (param2[1] == 1){ //infinity slope case
			edgePoint1.push_back({param1[0],PT_UL[1]});
			edgePoint2.push_back({param1[0],PT_BL[1]});
		}
		else if (param2[0] == 0){
			edgePoint1.push_back({PT_BL[0], param1[1]});
			edgePoint2.push_back({PT_BR[0], param1[1]});
		}
		else{
			slope = param2[0];
			yint = param1[1] - slope*param1[0];
		}
	}

	cv::vector<cv::Vec2f> candidates(4);
	cv::vector<cv::Vec2f> candidates2;
	cv::vector<cv::Vec2f> candidates3;
	if ((edgePoint1.size()==0)&&(edgePoint2.size()==0)){

		candidates[0] = { PT_UR[0], slope*PT_UR[0] + yint };
		candidates[1] = { (PT_UL[1]-yint)/slope, PT_UL[1] };
		candidates[2] = { PT_UL[0], slope*PT_UL[0] + yint };
		candidates[3] = { (PT_BR[1]-yint)/slope, PT_BR[1] };

		int i;
		for (i = 0; i < 4; i++){
			if ((candidates[i][0]<=PT_UR[0])&&(candidates[i][1]<=PT_UL[1])&&(candidates[i][0]>=PT_UL[0])&&(candidates[i][1]>=PT_BR[1])){
				candidates2.push_back(candidates[i]);
			}
		}

		for (i = 0; i < candidates2.size(); i++){
			if (((candidates2[i][0] != PT_UR[0]) && (candidates2[i][1] != PT_UR[1])) || ((candidates2[i][0] != PT_UL[0]) && (candidates2[i][1] != PT_UL[1])) || ((candidates2[i][0] != PT_BL[0]) && (candidates2[i][1] != PT_BL[1])) || ((candidates2[i][0] != PT_BR[0]) && (candidates2[i][1] != PT_BR[1]))){
				candidates3.push_back(candidates2[i]);
			}
		}

		if (candidates3.size() == 1){
			edgeIntPoint = candidates3;
		}
		else{
			edgePoint1.push_back(candidates3[0]);
			edgePoint2.push_back(candidates3[1]);
		}

	}

	if (edgeIntPoint.size() == 0){
		if (featureIndex==0){
			if (norm(edgePoint1[0],param1) < norm(edgePoint1[0],param2)){
				edgeIntPoint = edgePoint2;
			}
			else{
				edgeIntPoint = edgePoint1;
			}
		}
		else{
			if (norm(edgePoint1[0], param1)<norm(edgePoint2[0], param1)){
			edgeIntPoint = edgePoint2;
		}
		else{
			edgeIntPoint = edgePoint1;
		}
	}
	}

	return edgeIntPoint;

}

cv::vector<cv::Vec2f> PhysicsModel::pocketPointFromLine(int featureIndex, cv::Vec2f param1, cv::Vec2f param2, float radius){
	
	cv::vector<cv::Vec2f> pocketCoordinates(4);
	pocketCoordinates[0] = PT_UR;
	pocketCoordinates[1] = PT_UL;
	pocketCoordinates[2] = PT_BL;
	pocketCoordinates[3] = PT_BR;

	cv::vector<cv::Vec2f> pocketIntPoint;

	int i;
	cv::vector<cv::Vec2f> candidate;
	for (i = 0; i < 4; i++){
		candidate = backPointFromLine(featureIndex,param1,param2,pocketCoordinates[i],radius);
		if (candidate.size() != 0){
			pocketIntPoint = candidate;
		}
	}
	return pocketIntPoint;
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