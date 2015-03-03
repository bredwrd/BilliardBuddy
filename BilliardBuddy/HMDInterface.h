#pragma once

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

class HMDInterface
{
private:


public:
	HMDInterface();
	~HMDInterface();
	void HMDInterface::drawToHMD(cv::Mat& leftFrame, cv::Mat& rightFrame);

};
