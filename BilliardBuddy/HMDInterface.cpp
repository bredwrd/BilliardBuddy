#include "HMDInterface.h"

HMDInterface::HMDInterface()
{

}

HMDInterface::~HMDInterface()
{

}

void HMDInterface::drawToHMD(cv::Mat& leftFrame, cv::Mat& rightFrame)
{
	// Resize left and right frames.
	cv::resize(leftFrame, leftFrame, cv::Size(960,1080), 0, 0, cv::INTER_NEAREST);
	cv::resize(rightFrame, rightFrame, cv::Size(960, 1080), 2, 2, cv::INTER_NEAREST);

	// Merge left and right frames.
	cv::Mat mergedFrame;
	cv::hconcat(leftFrame, rightFrame, mergedFrame);

	// Display feed to user.
	imshow("Merged Feed", mergedFrame);
}