#pragma once
#include "VisualAugmentor.h"
class TextAugmentor :
	public VisualAugmentor
{
private:
	void textAugment(cv::Mat& frame, std::string msg);

public:
	TextAugmentor();
	~TextAugmentor();

	void augment(cv::Mat& frame);
};

