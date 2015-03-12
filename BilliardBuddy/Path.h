#pragma once

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/features2d/features2d.hpp"

#ifndef PATH_H
#define PATH_H

struct Path {
	cv::Vec2f startPoint;
	cv::Vec2f endPoint;
	/*Path();
	~Path();*/
};

#endif