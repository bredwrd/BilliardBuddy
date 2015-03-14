#pragma once

#include "VisualAugmentor.h"
#include "Path.h"

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

class TrajectoryAugmentor :
	public virtual VisualAugmentor
{
private:

public:
	TrajectoryAugmentor();
	~TrajectoryAugmentor();
	void TrajectoryAugmentor::augment(cv::Mat& frame, cv::vector<Path> trajectoryPoints); // points must have length of 2
};