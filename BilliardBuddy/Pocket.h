#pragma once

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/features2d/features2d.hpp"

#ifndef POCKET_H
#define POCKET_H

struct pocket {
	cv::KeyPoint pocketPoints;
	float xLocation = NULL;
	float yLocation = NULL;
	pocket();
	~pocket();
};

#endif