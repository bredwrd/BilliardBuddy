#pragma once

#include <opencv2/core/core.hpp>

class VisualAugmentor
{
public:
	VisualAugmentor();
	virtual ~VisualAugmentor();

	virtual void augment(cv::Mat& frame) = 0;
};

