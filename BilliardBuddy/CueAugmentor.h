#pragma once

#include "VisualAugmentor.h"

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

class CueAugmentor :
	public virtual VisualAugmentor
{
private:


public:
	CueAugmentor();
	~CueAugmentor();
	void CueAugmentor::augment(cv::Mat& frame, cv::vector<cv::Vec2i> points);

};

