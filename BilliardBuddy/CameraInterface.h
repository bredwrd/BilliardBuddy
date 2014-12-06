#pragma once

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "Settings.h"

class CameraInterface
{
public:
	CameraInterface(Settings& settings);
	~CameraInterface();

	void getLeftFrame(cv::Mat& frame);
	void getRightFrame(cv::Mat& frame);

private:
	cv::VideoCapture leftVideoCapture;
	cv::VideoCapture rightVideoCapture;
	cv::Mat leftFrame;
	cv::Mat rightFrame;

	void getFrame(cv::VideoCapture& captureSource, cv::Mat& frame);
	void initializeVideoSources(Settings& settings);
};

