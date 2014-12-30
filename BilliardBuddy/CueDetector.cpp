#include "CueDetector.h"

CueDetector::CueDetector()
{
	cueLine.resize(2);
}

CueDetector::~CueDetector()
{
}

cv::vector<cv::Vec2i> CueDetector::detect(cv::Mat frame)
{
	cv::Mat croppedFrame = frame.clone(); // copy so we don't overwrite the user's view with GaussianBlur's blur call.
	croppedFrame = croppedFrame(cv::Rect(CROP_X, CROP_Y, CROP_WIDTH, CROP_HEIGHT)); // Crop the image for the typical location of the cue.
	GaussianBlur(croppedFrame);
	hsiSegment(croppedFrame);
	skeleton(croppedFrame);
	//regHoughLines(croppedFrame, 120);
	probHoughLines(croppedFrame, 50, 80, 35);

	imshow("Cue view", croppedFrame);
	return cueLine;
}

void CueDetector::probHoughLines(cv::Mat& frame, int threshold, int minLength, int maxGap) {
	cv::vector<cv::Vec4i> lines;
	HoughLinesP(frame, lines, 1, CV_PI / 180, threshold, minLength, maxGap);

	cv::Mat houghMap;
	cvtColor(frame, houghMap, CV_GRAY2BGR);

	for (size_t i = 0; i < lines.size(); i++)
	{
		line(houghMap, cv::Point(lines[i][0], lines[i][1]), cv::Point(lines[i][2], lines[i][3]), cv::Scalar(0, 0, 255), 3, CV_AA);
	}

	if (lines.size() == 1) {
		cv::Vec4i l = lines[0];

		// Convert cropped coords to global coords.
		cueLine[0] = cv::Vec2i(lines[0][0] + CROP_X, lines[0][1] + CROP_Y);
		cueLine[1] = cv::Vec2i(lines[0][2] + CROP_X, lines[0][3] + CROP_Y);
	}

	frame = houghMap.clone();
}

void CueDetector::regHoughLines(cv::Mat& frame, int threshold) {
	cv::vector<cv::Vec2f> lines;
	HoughLines(frame, lines, 1, CV_PI / 180, threshold);

	// Prepare Hough map.
	cv::Mat houghMap;
	cvtColor(frame, houghMap, CV_GRAY2BGR);

	// Draw Hough lines.
	for (size_t i = 0; i < lines.size(); i++)
	{
		float rho = lines[i][0], theta = lines[i][1];
		cv::Point pt1, pt2;
		double a = cos(theta), b = sin(theta);
		double x0 = a*rho, y0 = b*rho;
		pt1.x = cvRound(x0 + 1000 * (-b));
		pt1.y = cvRound(y0 + 1000 * (a));
		pt2.x = cvRound(x0 - 1000 * (-b));
		pt2.y = cvRound(y0 - 1000 * (a));
		line(houghMap, pt1, pt2, cv::Scalar(0, 0, 255), 3, CV_AA);
	}

	frame = houghMap.clone();
}

void CueDetector::hsiSegment(cv::Mat& frame)
{
	//Can be used to control with trackbars the values
	int minH = 22;
	int maxH = 62;

	int minS = 14;
	int maxS = 166;

	int minV = 215;
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
}

void CueDetector::GaussianBlur(cv::Mat& frame)
{
	//int display_dst(int delay);
	for (int i = 1; i < 7; i = i + 2)
	{
		cv::blur(frame, frame, cv::Size(i, i), cv::Point(-1, -1));
	}
}

void CueDetector::skeleton(cv::Mat& frame)
{
	// http://stackoverflow.com/questions/16665742/a-good-approach-for-detecting-lines-in-an-image
	// http://felix.abecassis.me/2011/09/opencv-morphological-skeleton/

	cvtColor(frame, frame, cv::COLOR_BGR2GRAY); //Convert the captured frame from BGR to greyscale

	cv::threshold(frame, frame, 127, 255, cv::THRESH_BINARY);
	cv::Mat skel(frame.size(), CV_8UC1, cv::Scalar(0));
	cv::Mat temp;
	cv::Mat eroded;

	cv::Mat element = cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(3, 3));

	bool done;
	do
	{
		cv::erode(frame, eroded, element);
		cv::dilate(eroded, temp, element); // temp = open(img)
		cv::subtract(frame, temp, temp);
		cv::bitwise_or(skel, temp, skel);
		eroded.copyTo(frame);

		done = (cv::countNonZero(frame) == 0);
	} while (!done);

	frame = skel.clone();
}
