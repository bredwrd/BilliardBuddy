#pragma once

#include <opencv2/core/core.hpp>

class VisualAugmentor
{
public:
	VisualAugmentor();
	virtual ~VisualAugmentor();

	virtual void augment(cv::Mat& frame, cv::vector<cv::Vec2i> points); // length of points vector depends on application
};

