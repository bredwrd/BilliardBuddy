#pragma once

#include <sstream>
#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>


using namespace std;
using namespace cv;

class Settings
{
public:
	Settings() : goodInput(false) {}
	~Settings();

	enum Pattern { NOT_EXISTING, CHESSBOARD, CIRCLES_GRID, ASYMMETRIC_CIRCLES_GRID };
	enum InputType { INVALID, CAMERA, VIDEO_FILE, IMAGE_LIST };

	string outputFileName;      // The name of the file where to write
	bool showUndistorsed;       // Show undistorted images after calibration
	string rightInput;               // The input ->
	string leftInput;

	int rightCameraID;
	int leftCameraID;

	VideoCapture rightInputCapture;
	VideoCapture leftInputCapture;
	InputType inputType;
	bool goodInput;
	int flag;

	void read(const FileNode& node); //Read serialization for this class
	void interprate();
	Mat nextImage(VideoCapture& captureSource);
	static bool readStringList(const string& filename, vector<string>& l);

private:
	string patternToUse;
};