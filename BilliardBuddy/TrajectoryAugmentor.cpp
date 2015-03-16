#include "TrajectoryAugmentor.h"
#include "Path.h"

TrajectoryAugmentor::TrajectoryAugmentor()
{
}

TrajectoryAugmentor::~TrajectoryAugmentor()
{
}

void TrajectoryAugmentor::augment(cv::Mat& frame, cv::vector<Path> trajectoryPoints)
{
	//Augment the end points overtop of a frame in 3D
	for (int i = 0; i < trajectoryPoints.size(); i++){
		cv::Vec2f tempStart = trajectoryPoints[i].startPoint;
		cv::Vec2f tempEnd = trajectoryPoints[i].endPoint;
		cv::Vec2i tempStartInt;
		cv::Vec2i tempEndInt;
		tempStartInt[0] = (int)tempStart[0];
		tempStartInt[1] = (int)tempStart[1];
		tempEndInt[0] = (int)tempEnd[0];
		tempEndInt[1] = (int)tempEnd[1];
		if ((i==0)||(i==1)){
			cv::line(frame, cv::Point(tempStartInt[0], tempStartInt[1]), cv::Point(tempEndInt[0], tempEndInt[1]), cv::Scalar(255, 255, 255), 1, CV_AA);
		}
		else{
			cv::line(frame, cv::Point(tempStartInt[0], tempStartInt[1]), cv::Point(tempEndInt[0], tempEndInt[1]), cv::Scalar(0, 0, 255), 1, CV_AA);
		}
	
	}

}