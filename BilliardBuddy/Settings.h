#pragma once

#include <sstream>
#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

class Settings
{
public:
	Settings();
	~Settings();

	enum Pattern { NOT_EXISTING, CHESSBOARD, CIRCLES_GRID, ASYMMETRIC_CIRCLES_GRID };
	enum InputType { INVALID, CAMERA, VIDEO_FILE, IMAGE_LIST };

	std::string outputFileName;      // The name of the file where to write
	std::string rightInput;               // The input ->
	std::string leftInput;

	int rightCameraID;
	int leftCameraID;

	cv::VideoCapture rightInputCapture;
	cv::VideoCapture leftInputCapture;
	InputType inputType;
	bool goodInput;
	int flag;

	void read(const cv::FileNode& node); //Read serialization for this class
	void validate();
	static bool readStringList(const std::string& filename, cv::vector<std::string>& l);

	// Accessors & Mutators
	cv::Mat getCameraMatrix();
	cv::Mat getDistCoeffs();

private:
	std::string patternToUse;
	cv::Mat cameraMatrix;
	cv::Mat distCoeffs;

	void readApplicationParams(const cv::FileNode& node);
	void readCameraParams();

};