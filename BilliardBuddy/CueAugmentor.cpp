#include "CueAugmentor.h"

CueAugmentor::CueAugmentor()
{
}

CueAugmentor::~CueAugmentor()
{
}

void CueAugmentor::augment(cv::Mat& frame, cv::vector<cv::Vec2i> points)
{
	cv::line(frame, cv::Point(points[0][0], points[0][1]), cv::Point(points[1][0], points[1][1]), cv::Scalar(0, 0, 255), 3, CV_AA);
}