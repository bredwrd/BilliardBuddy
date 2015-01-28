#include "BallDetector.h"


BallDetector::BallDetector()
{
}


BallDetector::~BallDetector()
{
}

cv::vector<cv::Vec2i> BallDetector::detect(cv::Mat frame)
{
	// Reset cueLines.
	ballCoordinates.clear();

	// Convert image to greyscale.
	// Maybe in future take Hue channel instead of greyscale...
	cv::Mat greyscaleFrame;
	cv::cvtColor(frame, greyscaleFrame, CV_BGR2GRAY);

	// Detect circles.
	cv::vector<cv::Vec3f> ballCandidates;
	HoughCircles(greyscaleFrame, ballCandidates, CV_HOUGH_GRADIENT, 1, 4, 60, 30);

	// Draw circles on BGR frame.
	for (size_t i = 0; i < ballCandidates.size(); i++)
	{
		cv::Point center(cvRound(ballCandidates[i][0]), cvRound(ballCandidates[i][1]));
		int radius = cvRound(ballCandidates[i][2]);
		// draw the circle center
		circle(frame, center, 3, cv::Scalar(0, 255, 0), -1, 8, 0);
		// draw the circle outline
		circle(frame, center, radius, cv::Scalar(0, 0, 255), 3, 8, 0);
	}

	imshow("BallDetector", frame);

	return ballCoordinates;
}