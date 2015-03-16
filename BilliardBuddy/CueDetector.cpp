#include "CueDetector.h"

CueDetector::CueDetector()
{
	cueLine.resize(2);
}

CueDetector::~CueDetector()
{
}

cv::vector<cv::Vec2i> CueDetector::detect(cv::Mat frame, int frameIterator)
{
	// Reset cueLines.
	cueLine[0] = cv::Vec2i(0, 0);
	cueLine[1] = cv::Vec2i(0, 0);

	cv::Mat croppedFrame = frame.clone(); // copy so we don't overwrite the user's view with GaussianBlur's blur call.
	croppedFrame = croppedFrame(cv::Rect(CROP_X, CROP_Y, CROP_WIDTH, CROP_HEIGHT)); // Crop the image for the typical location of the cue.
	imshow("Debug cropped cue", croppedFrame);
	GaussianBlur(croppedFrame);
	hsiSegment(croppedFrame);
	skeleton(croppedFrame);
	//regHoughLines(croppedFrame, 120);
	probHoughLinesCueSegments(croppedFrame);

	cv::line(frame, cv::Point(cueLine[0][0], cueLine[0][1]), cv::Point(cueLine[1][0], cueLine[1][1]), cv::Scalar(0, 0, 255), 3, CV_AA);

	// Assumption: second cueLine point is always the uppermost.
	CueBallDetector cueBallDetector;
	if (cueLine[1][1] > cueLine[0][1])
	{
		cueBallDetector.setCropX(cueLine[1][0]);
		cueBallDetector.setCropY(cueLine[1][1]);
	}
	else
	{
		cueBallDetector.setCropX(cueLine[0][0]);
		cueBallDetector.setCropY(cueLine[0][1]);
	}

	cueBallDetector.detect(frame, frameIterator);

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

	//imshow("Cue Segments", houghMap);

	frame = houghMap.clone();
}

void CueDetector::mergeCueSegments(cv::Mat& frame)
{
	// Combine multiple cue segments into a single cue candidate.
	cv::vector<cv::Vec4i> lines;
	HoughLinesP(frame, lines , 1, CV_PI / 180, 50, 40, 60);

	if (lines.size() > 0)
	{
		// Convert vector 'lines' to Mat 'points'.

		// Declare startPoints and endPoints to hold respective points of detected HoughLines.
		cv::Mat startPoints(lines.size(), 2, CV_32FC1);
		cv::Mat endPoints(lines.size(), 2, CV_32FC1);

		// Declare allPoints to be the vertical concatenation of startPoints and endPoints.
		// Twice the length of lines because HoughLines contain two points.
		cv::Mat allPoints(lines.size()*2, 2, CV_32FC1);
		allPoints = cv::Scalar(0);

		// Convert n-length vector of 4-integer length vector of line boundary points to nx2 Mats.
		for (int i = 0; i < lines.size(); i = i++)
		{
			for (int j = 0; j <= 1; j++)
			{
				startPoints.at<float>(i, j) = lines[i][j];
				endPoints.at<float>(i, j) = lines[i][j+2]; // endpoints are returned from HoughLines as the last two elements in a row.
			}
		}

		// Vertically concatenate startPoints and endPoints (undocumented, see http://answers.opencv.org/question/1368/concatenating-matrices/)
		cv::vconcat(startPoints, endPoints, allPoints);

		// Calculate the mean cue endpoints using K=2-means clustering. See http://www.aishack.in/tutorials/kmeans-clustering-in-opencv/
		int K = 2;
		cv::Mat labels;
		cv::Mat centers(K, 2, allPoints.type());
		cv::kmeans(allPoints, K, labels, cv::TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 10, 1.0), 50, cv::KMEANS_PP_CENTERS, centers);

		// Convert cropped coords to global coords.
		cueLine[0] = cv::Vec2i(centers.at<float>(0, 0) + CROP_X, centers.at<float>(0, 1) + CROP_Y);
		cueLine[1] = cv::Vec2i(centers.at<float>(1, 0) + CROP_X, centers.at<float>(1, 1) + CROP_Y);
	}
	else // no lines detected
	{
		// Set cueLines to initial (negligable) values.
		cueLine[0] = cv::Vec2i(0, 0);
		cueLine[1] = cv::Vec2i(0, 0);
	}
}

void CueDetector::hsiSegment(cv::Mat& frame)
{
	//Can be used to control with trackbars the values
	int minH = 14;
	int maxH = 57;

	int minS = 155;
	int maxS = 215;

	int minV = 50;
	int maxV = 150;

	// Downsample before hsi segmentation
	cv::Mat downsampledFrame;
	cv::resize(frame, downsampledFrame, cv::Size(0, 0), 0.5, 0.5, cv::INTER_CUBIC);

	// Convert the captured frame from BGR to HSV
	cv::Mat hsvFrame;
	cvtColor(downsampledFrame, hsvFrame, cv::COLOR_BGR2HSV); 

	// Threshold the image
	cv::Mat hsvMask;
	inRange(hsvFrame, cv::Scalar(minH, minS, minV), cv::Scalar(maxH, maxS, maxV), hsvMask);

	// Upsample after hsi segmentation
	cv::resize(hsvMask, hsvMask, cv::Size(0, 0), HSI_SEGMENTATION_DOWNSAMPLE_FACTOR, HSI_SEGMENTATION_DOWNSAMPLE_FACTOR, cv::INTER_CUBIC);

	// Used to make the mask bigger
	dilate(hsvMask, hsvMask, getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(4, 4)));

	cv::Mat maskedFrame;
	frame.copyTo(maskedFrame, hsvMask);
	frame = maskedFrame.clone();

	//imshow("HSI Cue", frame);
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

	cv::threshold(frame, frame, 1, 255, cv::THRESH_BINARY);
	cv::Mat skel(frame.size(), CV_8UC1, cv::Scalar(0));
	cv::Mat temp;
	cv::Mat eroded;

	cv::Mat element = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));

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