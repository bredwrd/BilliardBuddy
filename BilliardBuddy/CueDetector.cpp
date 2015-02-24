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
	HoughLinesP(frame, lines , 1, CV_PI / 180, 50, 18, 100);

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
				// Debug messages
				std::cout << "startPoint (" << i << ", " << j << ") = " << lines[i][j] << std::endl;
				startPoints.at<float>(i, j) = lines[i][j];

				std::cout << "endPoint (" << i << ", " << j << ") = " << lines[i][j+2] << std::endl;
				endPoints.at<float>(i, j) = lines[i][j+2];
			}
		}

		// Vertically concatenate startPoints and endPoints (undocumented, see http://answers.opencv.org/question/1368/concatenating-matrices/)
		cv::vconcat(startPoints, endPoints, allPoints);

		// Debug messages
		std::cout << "startPoints = " << startPoints << std::endl;
		std::cout << "endPoints = " << endPoints << std::endl;
		std::cout << "points = " << allPoints << std::endl;

		// Calculate the mean cue endpoints using K=2-means clustering. See http://www.aishack.in/tutorials/kmeans-clustering-in-opencv/
		int K = 2;
		cv::Mat labels;
		cv::Mat centers(K, 2, allPoints.type());
		cv::kmeans(allPoints, K, labels, cv::TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 10, 1.0), 50, cv::KMEANS_PP_CENTERS, centers);

		// Debug messages for point classifications
		for (int i = 0; i < 2*lines.size(); i++)
		{
			int clusterIdx = labels.at<int>(i);
			std::cout << "point " << i << " = " << "class " << clusterIdx << std::endl;
			
		}


		// Debug messages for class centres
		for (int i = 0; i < 2; i++)
		{
			for (int j = 0; j < 2; j++)
			{
				int cell = centers.at<float>(i, j);
				std::cout << "center (" << i << ", " << j << ") = " << cell << std::endl;
			}
		}

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

	// Debug video display.
	cv::Mat houghMap(frame.size(), CV_8UC3, cv::Scalar(0));
	cv::Vec4i cueCandidatesSum;
	cueCandidatesSum = cv::Vec4i(0, 0, 0, 0);
	for (size_t i = 0; i < lines.size(); i++)
	{
		line(houghMap, cv::Point(lines[i][0], lines[i][1]), cv::Point(lines[i][2], lines[i][3]), cv::Scalar(255, 255, 255), 1, CV_AA);
	}

	float cueCandidatesMean[4];
	for (int i = 0; i < 4; i++)
	{
		cueCandidatesMean[i] = cueCandidatesSum[i] / lines.size();
	}

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
	//includes the whole pool table, having it a bit bigger thant he pool table is not an issue.
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

void CueDetector::kmeansDemo()
{
	const int MAX_CLUSTERS = 5;
	cv::Scalar colorTab[] =
	{
		cv::Scalar(0, 0, 255),
		cv::Scalar(0, 255, 0),
		cv::Scalar(255, 100, 100),
		cv::Scalar(255, 0, 255),
		cv::Scalar(0, 255, 255)
	};

	cv::Mat img(500, 500, CV_8UC3);
	cv::RNG rng(12345);

	for (;;)
	{
		int k, clusterCount = rng.uniform(2, MAX_CLUSTERS + 1);
		int i, sampleCount = rng.uniform(1, 1001);
		cv::Mat points(sampleCount, 2, CV_32F), labels;

		clusterCount = MIN(clusterCount, sampleCount);
		cv::Mat centers;

		/* generate random sample from multigaussian distribution */
		for (k = 0; k < clusterCount; k++)
		{
			cv::Point center;
			center.x = rng.uniform(0, img.cols);
			center.y = rng.uniform(0, img.rows);
			cv::Mat pointChunk = points.rowRange(k*sampleCount / clusterCount,
				k == clusterCount - 1 ? sampleCount :
				(k + 1)*sampleCount / clusterCount);
			rng.fill(pointChunk, CV_RAND_NORMAL, cv::Scalar(center.x, center.y), cv::Scalar(img.cols*0.05, img.rows*0.05));
		}

		randShuffle(points, 1, &rng);

		kmeans(points, clusterCount, labels,
			cv::TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 10, 1.0),
			3, cv::KMEANS_PP_CENTERS, centers);

		img = cv::Scalar::all(0);

		for (i = 0; i < sampleCount; i++)
		{
			int clusterIdx = labels.at<int>(i);
			cv::Point ipt = points.at<cv::Point2f>(i);
			circle(img, ipt, 2, colorTab[clusterIdx], CV_FILLED, CV_AA);
		}

		imshow("clusters", img);

		char key = (char)cv::waitKey();
		if (key == 27 || key == 'q' || key == 'Q') // 'ESC'
			break;
	}
}