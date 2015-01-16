#include "CueBallDetector.h"

CueBallDetector::CueBallDetector()
{
	cueBallCentre.resize(1);
}

CueBallDetector::~CueBallDetector()
{
}

cv::vector<cv::Vec2i> CueBallDetector::detect(cv::Mat frame)
{
	// Reset cueBallCentre.
	cueBallCentre[0] = cv::Vec2i(0, 0);
	cueBallDiameter = 0;

	cv::Mat modifiedFrame = frame.clone(); // copy so we don't overwrite the user's view with GaussianBlur's blur call.
	//cv::GaussianBlur(modifiedFrame, modifiedFrame, cv::Size(5,5), 7, 0, 4);


	hsiSegment(modifiedFrame);

	return cueBallCentre;
}


void CueBallDetector::hsiSegment(cv::Mat& frame)
{
	//Can be used to control with trackbars the values
	int minH = 1;
	int maxH = 100;

	int minS = 1;
	int maxS = 70;

	int minV = 245;
	int maxV = 255;

	cv::Mat hsvFrame;

	cvtColor(frame, hsvFrame, cv::COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV

	cv::Mat hsvMask;

	inRange(hsvFrame, cv::Scalar(minH, minS, minV), cv::Scalar(maxH, maxS, maxV), hsvMask); //Threshold the image

	//Used to make the mask bigger (For our specific situation we want to make sure the mask
	//includes the whole pool table, having it a bit bigger thant he pool table is not an issue.
	dilate(hsvMask, hsvMask, getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(4, 4)));

	cv::Mat maskedFrame;
	frame.copyTo(maskedFrame, hsvMask);
	frame = maskedFrame.clone();

	imshow("HSI Cue Ball", frame);
}

void CueBallDetector::gaussianBlur(cv::Mat& frame)
{
	//int display_dst(int delay);
	for (int i = 1; i < 7; i = i + 2)
	{
		cv::blur(frame, frame, cv::Size(i, i), cv::Point(-1, -1));
	}
}