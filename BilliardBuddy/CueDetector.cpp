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
	// Reset cueLines.
	cueLine[0] = cv::Vec2i(0, 0);
	cueLine[1] = cv::Vec2i(0, 0);

	cv::Mat croppedFrame = frame.clone(); // copy so we don't overwrite the user's view with GaussianBlur's blur call.
	croppedFrame = croppedFrame(cv::Rect(CROP_X, CROP_Y, CROP_WIDTH, CROP_HEIGHT)); // Crop the image for the typical location of the cue.
	GaussianBlur(croppedFrame);
	hsiSegment(croppedFrame);
	skeleton(croppedFrame);
	//regHoughLines(croppedFrame, 120);
	probHoughLinesCueSegments(croppedFrame);

	cv::line(frame, cv::Point(cueLine[0][0], cueLine[0][1]), cv::Point(cueLine[1][0], cueLine[1][1]), cv::Scalar(0, 0, 255), 3, CV_AA);

	return cueLine;
}

void CueDetector::probHoughLinesCueSegments(cv::Mat& frame) {
	// Finds segments of a cue interrupted by a hand.
	cv::vector<cv::Vec4i> lines;
	HoughLinesP(frame, lines, 1, CV_PI / 180, HOUGH_THRESHOLD, CUE_SEGMENT_MIN_LENGTH, CUE_SEGMENT_MAX_GAP);

	cv::Mat houghMap(frame.size(), CV_8UC1, cv::Scalar(0));

	for (size_t i = 0; i < lines.size(); i++)
	{
		line(houghMap, cv::Point(lines[i][0], lines[i][1]), cv::Point(lines[i][2], lines[i][3]), cv::Scalar(255), 1, CV_AA);
	}

	mergeCueSegments(houghMap);

	imshow("Cue Segments", houghMap);

	frame = houghMap.clone();
}

void CueDetector::mergeCueSegments(cv::Mat& frame)
{
	// combines two cue segments into a single line segment.
	cv::vector<cv::Vec4i> lines;
	HoughLinesP(frame, lines, 1, CV_PI / 180, 50, 18, 100);

	// Calculate the mean cue endpoints.
	cv::Mat houghMap(frame.size(), CV_8UC3, cv::Scalar(0));
	cv::Vec4i cueCandidatesSum;
	cueCandidatesSum = cv::Vec4i(0, 0, 0, 0);
	for (size_t i = 0; i < lines.size(); i++)
	{
		line(houghMap, cv::Point(lines[i][0], lines[i][1]), cv::Point(lines[i][2], lines[i][3]), cv::Scalar(255, 255, 255), 1, CV_AA);
		cueCandidatesSum += cv::Vec4i(lines[i][0], lines[i][1], lines[i][2], lines[i][3]);
	}
	float cueCandidatesMean[4];
	for (int i = 0; i < 4; i++)
	{
		cueCandidatesMean[i] = cueCandidatesSum[i] / lines.size();
	}

	// Convert cropped coords to global coords.
	cueLine[0] = cv::Vec2i(cueCandidatesMean[0] + CROP_X, cueCandidatesMean[1] + CROP_Y);
	cueLine[1] = cv::Vec2i(cueCandidatesMean[2] + CROP_X, cueCandidatesMean[3] + CROP_Y);

	imshow("Cue Lines", houghMap);
}

void CueDetector::hsiSegment(cv::Mat& frame)
{
	//Can be used to control with trackbars the values
	int minH = 10;
	int maxH = 40;

	int minS = 1;
	int maxS = 170;

	int minV = 175;
	int maxV = 255;

	cv::Mat hsvFrame;

	cvtColor(frame, hsvFrame, cv::COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV

	cv::Mat hsvMask;

	inRange(hsvFrame, cv::Scalar(minH, minS, minV), cv::Scalar(maxH, maxS, maxV), hsvMask); //Threshold the image

	//Used to make the mask bigger (For our specific situation we want to make sure the mask
	//includes the whole pool table, having it a bit bigger than the pool table is not an issue.
	dilate(hsvMask, hsvMask, getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(4, 4)));

	cv::Mat maskedFrame;
	frame.copyTo(maskedFrame, hsvMask);
	frame = maskedFrame.clone();

	imshow("HSI Cue", frame);
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
