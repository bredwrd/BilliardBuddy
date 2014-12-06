#pragma once

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "Settings.h"

class CameraInterface
{
public:
	CameraInterface(Settings& settings);
	~CameraInterface();

	void getFrames(cv::Mat& leftFrame, cv::Mat& rightFrame);

private:
	cv::VideoCapture leftVideoCapture;
	cv::VideoCapture rightVideoCapture;
	cv::Mat leftFrame;
	cv::Mat rightFrame;

	void grabFrames();
	void retrieveFrames(cv::Mat& leftFrame, cv::Mat& rightFrame);
	void initializeVideoSources(Settings& settings);
};

