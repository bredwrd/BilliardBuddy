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
class Settings
{
public:
	Settings() : goodInput(false) {}
	enum Pattern { NOT_EXISTING, CHESSBOARD, CIRCLES_GRID, ASYMMETRIC_CIRCLES_GRID };
	enum InputType { INVALID, CAMERA, VIDEO_FILE, IMAGE_LIST };

	void read(const FileNode& node)                          //Read serialization for this class
	{
		node["BoardSize_Width"] >> boardSize.width;
		node["BoardSize_Height"] >> boardSize.height;
		node["Calibrate_Pattern"] >> patternToUse;
		node["Square_Size"] >> squareSize;
		node["Calibrate_NrOfFrameToUse"] >> nrFrames;
		node["Calibrate_FixAspectRatio"] >> aspectRatio;
		node["Write_DetectedFeaturePoints"] >> bwritePoints;
		node["Write_extrinsicParameters"] >> bwriteExtrinsics;
		node["Write_outputFileName"] >> outputFileName;
		node["Calibrate_AssumeZeroTangentialDistortion"] >> calibZeroTangentDist;
		node["Calibrate_FixPrincipalPointAtTheCenter"] >> calibFixPrincipalPoint;
		node["Input_FlipAroundHorizontalAxis"] >> flipVertical;
		node["Show_UndistortedImage"] >> showUndistorsed;
		node["RightInput"] >> rightInput;
		node["LeftInput"] >> leftInput;
		node["Input_Delay"] >> delay;
		interprate();
	}
	void interprate()
	{
		goodInput = true;
		if (boardSize.width <= 0 || boardSize.height <= 0)
		{
			cerr << "Invalid Board size: " << boardSize.width << " " << boardSize.height << endl;
			goodInput = false;
		}
		if (squareSize <= 10e-6)
		{
			cerr << "Invalid square size " << squareSize << endl;
			goodInput = false;
		}
		if (nrFrames <= 0)
		{
			cerr << "Invalid number of frames " << nrFrames << endl;
			goodInput = false;
		}

		if (rightInput.empty() || leftInput.empty()) {     // Check for valid input
			inputType = INVALID;
		}
		else
		{
			if (rightInput[0] >= '0' && leftInput[0] <= '9')
			{
				stringstream ssRight(rightInput);
				stringstream ssLeft(leftInput);
				ssRight >> rightCameraID;
				ssLeft >> leftCameraID;
				inputType = CAMERA;
			}
			else
			{
				if (false)
				{
					inputType = IMAGE_LIST;
					nrFrames = (nrFrames < (int)imageList.size()) ? nrFrames : (int)imageList.size();
				}
				else
					inputType = VIDEO_FILE;
			}
			if (inputType == CAMERA) {
				rightInputCapture.open(rightCameraID);
				leftInputCapture.open(leftCameraID);
			}
			else if (inputType == VIDEO_FILE) {
				rightInputCapture.open(rightInput);
				leftInputCapture.open(leftInput);
			}
			else if (inputType != IMAGE_LIST && !rightInputCapture.isOpened())
				inputType = INVALID;
		}
		if (inputType == INVALID)
		{
			cerr << " Inexistent input: " << rightInput;
			goodInput = false;
		}

		flag = 0;
		if (calibFixPrincipalPoint) flag |= CV_CALIB_FIX_PRINCIPAL_POINT;
		if (calibZeroTangentDist)   flag |= CV_CALIB_ZERO_TANGENT_DIST;
		if (aspectRatio)            flag |= CV_CALIB_FIX_ASPECT_RATIO;


		calibrationPattern = NOT_EXISTING;
		if (!patternToUse.compare("CHESSBOARD")) calibrationPattern = CHESSBOARD;
		if (!patternToUse.compare("CIRCLES_GRID")) calibrationPattern = CIRCLES_GRID;
		if (!patternToUse.compare("ASYMMETRIC_CIRCLES_GRID")) calibrationPattern = ASYMMETRIC_CIRCLES_GRID;
		if (calibrationPattern == NOT_EXISTING)
		{
			cerr << " Inexistent camera calibration mode: " << patternToUse << endl;
			goodInput = false;
		}
		atImageList = 0;

	}
	Mat nextImage(VideoCapture& captureSource)
	{
		Mat result;
		if (captureSource.isOpened())
		{
			Mat view0;
			captureSource >> view0;
			view0.copyTo(result);
		}
		else if (atImageList < (int)imageList.size())
			result = imread(imageList[atImageList++], CV_LOAD_IMAGE_COLOR);

		return result;
	}

	static bool readStringList(const string& filename, vector<string>& l)
	{
		l.clear();
		FileStorage fs(filename, FileStorage::READ);
		if (!fs.isOpened())
			return false;
		FileNode n = fs.getFirstTopLevelNode();
		if (n.type() != FileNode::SEQ)
			return false;
		FileNodeIterator it = n.begin(), it_end = n.end();
		for (; it != it_end; ++it)
			l.push_back((string)*it);
		return true;
	}
public:
	Size boardSize;            // The size of the board -> Number of items by width and height
	Pattern calibrationPattern;// One of the Chessboard, circles, or asymmetric circle pattern
	float squareSize;          // The size of a square in your defined unit (point, millimeter,etc).
	int nrFrames;              // The number of frames to use from the input for calibration
	float aspectRatio;         // The aspect ratio
	int delay;                 // In case of a video input
	bool bwritePoints;         //  Write detected feature points
	bool bwriteExtrinsics;     // Write extrinsic parameters
	bool calibZeroTangentDist; // Assume zero tangential distortion
	bool calibFixPrincipalPoint;// Fix the principal point at the center
	bool flipVertical;          // Flip the captured images around the horizontal axis
	string outputFileName;      // The name of the file where to write
	bool showUndistorsed;       // Show undistorted images after calibration
	string rightInput;               // The input ->
	string leftInput;

	int rightCameraID;
	int leftCameraID;
	vector<string> imageList;
	int atImageList;
	VideoCapture rightInputCapture;
	VideoCapture leftInputCapture;
	InputType inputType;
	bool goodInput;
	int flag;

private:
	string patternToUse;


};

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

		if (s.flipVertical) {
			flip(rightView, rightView, 0);
			flip(leftView, leftView, 0);
		}

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

			// detect edges (Canny)
			Canny(rightView, rightView, 120, 180, 3, true);

			cvtColor(rightView, rightHoughMap, CV_GRAY2BGR);
			vector<Vec2f> rightLines;
			HoughLines(rightView, rightLines, 1, CV_PI/180, 100);

			// draw lines
			for (size_t i = 0; i < rightLines.size(); i++)
			{
				float rho = rightLines[i][0], theta = rightLines[i][1];
				Point pt1, pt2;
				double a = cos(theta), b = sin(theta);
				double x0 = a*rho, y0 = b*rho;
				pt1.x = cvRound(x0 + 1000 * (-b));
				pt1.y = cvRound(y0 + 1000 * (a));
				pt2.x = cvRound(x0 - 1000 * (-b));
				pt2.y = cvRound(y0 - 1000 * (a));
				line(rightHoughMap, pt1, pt2, Scalar(0, 0, 255), 3, CV_AA);
			}
			imshow("Right View", rightHoughMap);
			imshow("Left View", leftView);
		}
		else {
			//------------------------------ Show image and check for input commands -------------------
			imshow("Right View", rightView);
			imshow("Left View", leftView);
		}

		char key = (char)waitKey(s.rightInputCapture.isOpened() ? 50 : s.delay);


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
