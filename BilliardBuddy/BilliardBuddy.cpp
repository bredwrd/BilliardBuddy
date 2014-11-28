#ifndef _CRT_SECURE_NO_WARNINGS
# define _CRT_SECURE_NO_WARNINGS
#endif

#include <iostream>
#include <sstream>
#include <time.h>
#include <stdio.h>

#include <chrono>
#include <thread>

#include <opencv2/core/core.hpp>
#include <opencv2/ocl/ocl.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "Settings.h"

using namespace cv;
using namespace ocl;
using namespace std;

static void help()
{
	cout << "This is a camera calibration sample." << endl
		<< "Usage: calibration configurationFile" << endl
		<< "Near the sample file you'll find the configuration file, which has detailed help of "
		"how to edit it.  It may be any OpenCV supported file format XML/YAML." << endl;
}

static void read(const FileNode& node, Settings& x, const Settings& default_value = Settings())
{
	if (node.empty())
		x = default_value;
	else
		x.read(node);
}

enum { DETECTION = 0, CAPTURING = 1, CALIBRATED = 2 };

void loadCameraParams(Settings& s, Mat& cameraMatrix, Mat& distCoeffs);
void getStereoVideoFeed(Settings& s);
void oclUndistort(oclMat& gpu_temp, oclMat& gpu_view, oclMat& gpu_map1, oclMat& gpu_map2, Mat& view, Size& imageSize, Mat& cameraMatrix, Mat& distCoeffs);
void regHoughLines(Mat& view, Mat& houghMap, int threshold);
void probHoughLines(Mat& view, Mat& houghMap, int threshold, int minLineLength, int maxLineGap);

int main(int argc, char* argv[])
{
	help();
	//std::this_thread::sleep_for(std::chrono::milliseconds(8000)); // wait for 8 seconds to attach debugger
	Settings s;
	const string inputSettingsFile = argc > 1 ? argv[1] : "default.xml";
	FileStorage fs(inputSettingsFile, FileStorage::READ); // Read the settings
	if (!fs.isOpened())
	{
		cout << "Could not open the configuration file: \"" << inputSettingsFile << "\"" << endl;
		return -1;
	}
	fs["Settings"] >> s;
	fs.release();  // close Settings file

	if (!s.goodInput)
	{
		cout << "Invalid input detected. Application stopping. " << endl;
		return -1;
	}

	getStereoVideoFeed(s);

	return 0;
}

void getStereoVideoFeed(Settings& s) {
	vector<vector<Point2f> > imagePoints;
	Mat cameraMatrix, distCoeffs; // Use 
	Size imageSize, rotImageSize;
	int mode = s.inputType == Settings::IMAGE_LIST ? CAPTURING : DETECTION;
	loadCameraParams(s, cameraMatrix, distCoeffs);
	oclMat gpu_temp, gpu_view, gpu_map1, gpu_map2;
	mode = CALIBRATED;
	clock_t prevTimestamp = 0;
	const Scalar RED(0, 0, 255), GREEN(0, 255, 0);
	const char ESC_KEY = 27;
	int screenshotIndex = 1;
	for (int i = 0;; ++i)
	{
		Mat rightView;
		Mat leftView;
		bool blinkOutput = false;

		rightView = s.nextImage(s.rightInputCapture);
		leftView = s.nextImage(s.leftInputCapture);

		// Format input image.
		imageSize = rightView.size();
		rotImageSize = Size(imageSize.height, imageSize.width);

		vector<Point2f> pointBuf;

		//----------------------------- Output Text ------------------------------------------------
		string msg = (mode == CAPTURING) ? "100/100" :
			mode == CALIBRATED ? "Calibrated" : "Press 'g' to start";
		int baseLine = 0;
		Size textSize = getTextSize(msg, 1, 1, 1, &baseLine);
		Point textOrigin(rightView.cols - 2 * textSize.width - 10, rightView.rows - 2 * baseLine - 10);

		putText(rightView, msg, textOrigin, 1, 1, mode == CALIBRATED ? GREEN : RED);
		putText(leftView, msg, textOrigin, 1, 1, mode == CALIBRATED ? GREEN : RED);

		if (blinkOutput) {
			bitwise_not(rightView, rightView);
			bitwise_not(leftView, leftView);
		}
		Mat rightHoughMap;
		Mat leftHoughMap;
		//------------------------- Video capture  output  undistorted ------------------------------
		if (mode == CALIBRATED && s.showUndistorsed)
		{
			// undistort
			oclUndistort(gpu_temp, gpu_view, gpu_map1, gpu_map2, rightView, imageSize, cameraMatrix, distCoeffs);
			oclUndistort(gpu_temp, gpu_view, gpu_map1, gpu_map2, leftView, imageSize, cameraMatrix, distCoeffs);

			// orient images
			transpose(rightView, rightView);
			transpose(leftView, leftView);
			flip(rightView, rightView, 1);
			flip(leftView, leftView, 1);

			//Obtaining the Hough Map when it is original image in BGR format
			Mat imgOriginal = rightView;

			// detect edges (Canny)
			// Vary threshold1 and threshold2 parameters
			Canny(rightView, rightView, 100, 180, 3, true);
			Canny(leftView, leftView, 100, 180, 3, true);

			cvtColor(rightView, rightHoughMap, CV_GRAY2BGR);
			cvtColor(leftView, leftHoughMap, CV_GRAY2BGR);
			
			// Detect lines (regular HoughLines or probabilistic HoughLinesP)
			// Vary threshold parameter and with HouphLinesP, minLineLength and maxLineGap
			// Uncomment ONE and only ONE of regHoughLines or probHoughLines per view... or else!
			// The supposed advantage of HoughLinesP is much increased speed for slightly decreased accuracy.
			//regHoughLines(rightView, rightHoughMap, 100);
			probHoughLines(rightView, rightHoughMap, 80, 20, 11);
			//regHoughLines(leftView, leftHoughMap, 100);
			probHoughLines(leftView, leftHoughMap, 80, 20, 20);

			imshow("Right View", rightHoughMap);
			imshow("Left View", leftHoughMap);

			//Start of Colour Segmentation
			//Can be used to control with trackbars the values
			//namedWindow("Control", CV_WINDOW_AUTOSIZE); //create a window called "Control"

			int iLowH = 38;
			int iHighH = 130;

			int iLowS = 40;
			int iHighS = 200;

			int iLowV = 30;
			int iHighV = 200;

			//Create trackbars in "Control" window
			/*cvCreateTrackbar("LowH", "Control", &iLowH, 179); //Hue (0 - 179)
			cvCreateTrackbar("HighH", "Control", &iHighH, 179);

			cvCreateTrackbar("LowS", "Control", &iLowS, 255); //Saturation (0 - 255)
			cvCreateTrackbar("HighS", "Control", &iHighS, 255);

			cvCreateTrackbar("LowV", "Control", &iLowV, 255); //Value (0 - 255)
			cvCreateTrackbar("HighV", "Control", &iHighV, 255);
			*/

			//Inserted this line above to where rightView is formatted
			//Mat imgOriginal=rightView;

			Mat imgHSV;

			cvtColor(imgOriginal, imgHSV, COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV

			Mat imgThresholded;

			inRange(imgHSV, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgThresholded); //Threshold the image

			//morphological opening (remove small objects from the foreground)
			erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(8, 8)));
			dilate(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(8, 8)));

			//morphological closing (fill small holes in the foreground)
			dilate(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(8, 8)));
			erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(8, 8)));

			//Used to make the mask bigger (For our specific situation we want to make sure the mask
			//includes the whole pool table, having it a bit bigger thant he pool table is not an issue.
			dilate(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(30, 30)));

			imshow("Thresholded Image", imgThresholded); //show the thresholded image
			imshow("Original", imgOriginal); //show the original image

			//End of Colour Segmentation
		}
		else {
			//------------------------------ Show image and check for input commands -------------------
			imshow("Right View", rightView);
			imshow("Left View", leftView);
		}

		char key = (char)waitKey(20);


		if (key == ESC_KEY)
			break;

		if (key == 'u' && mode == CALIBRATED)
			s.showUndistorsed = !s.showUndistorsed;

		if (key == 's') {
			string rFileName = "results/right_" + to_string(screenshotIndex) + string(".tiff");
			string lFileName = "results/left_" + to_string(screenshotIndex) + string(".tiff");
			imwrite(rFileName, rightView);
			imwrite(lFileName, leftView);
			screenshotIndex++;
		}

		if (s.rightInputCapture.isOpened() && key == 'g')
		{
			mode = CAPTURING;
			imagePoints.clear();
		}
	}
}

void probHoughLines(Mat& view, Mat& houghMap, int threshold, int minLineLength, int maxLineGap) {
	vector<Vec4i> lines;
	HoughLinesP(view, lines, 1, CV_PI / 180, threshold, minLineLength, maxLineGap);

	for (size_t i = 0; i < lines.size(); i++)
	{
		Vec4i l = lines[i];
		line(houghMap, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0, 0, 255), 3, CV_AA);
	}
}

void regHoughLines(Mat& view, Mat& houghMap, int threshold) {
	vector<Vec2f> lines;
	HoughLines(view, lines, 1, CV_PI / 180, threshold);

	// draw lines
	for (size_t i = 0; i < lines.size(); i++)
	{
		float rho = lines[i][0], theta = lines[i][1];
		Point pt1, pt2;
		double a = cos(theta), b = sin(theta);
		double x0 = a*rho, y0 = b*rho;
		pt1.x = cvRound(x0 + 1000 * (-b));
		pt1.y = cvRound(y0 + 1000 * (a));
		pt2.x = cvRound(x0 - 1000 * (-b));
		pt2.y = cvRound(y0 - 1000 * (a));
		line(houghMap, pt1, pt2, Scalar(0, 0, 255), 3, CV_AA);
	}
}


void oclUndistort(oclMat& gpu_temp, oclMat& gpu_view, oclMat& gpu_map1, oclMat& gpu_map2, Mat& view, Size& imageSize, Mat& cameraMatrix, Mat& distCoeffs) {
	if (true) {
		Mat temp = view.clone();
		InputArray R = cv::noArray();
		Mat map1 = cv::Mat(), map2 = Mat();
		initUndistortRectifyMap(cameraMatrix, distCoeffs, R, cameraMatrix, imageSize, CV_32FC1, map1, map2);

		// Load matricies as ocl-compatible matricies
		gpu_temp.upload(temp);
		gpu_view.upload(view);
		gpu_map1.upload(map1);
		gpu_map2.upload(map2);

		ocl::remap(gpu_temp, gpu_view, gpu_map1, gpu_map2, INTER_NEAREST, BORDER_CONSTANT, 0);

		// Unload (only remap-prerequisite) matricies as CPU-formatted OpenCV matricies
		gpu_view.download(view);
	}
	else {
		Mat temp = view.clone();
		undistort(temp, view, cameraMatrix, distCoeffs);
	}

}

// Load the specified calibration file and store camera matrix and distortion coefficients matrix
void loadCameraParams(Settings& s, Mat& cameraMatrix, Mat& distCoeffs)
{
	FileStorage fs(s.outputFileName, FileStorage::READ);
	fs["Camera_Matrix"] >> cameraMatrix;
	fs["Distortion_Coefficients"] >> distCoeffs;
	fs.release();
}
