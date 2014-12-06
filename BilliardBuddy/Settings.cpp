#include "Settings.h"

Settings::Settings() : goodInput(false)
{
	//read();
}

Settings::~Settings()
{

}

void Settings::read(const cv::FileNode& node) //Read serialization for this class
{
	readApplicationParams(node);
	readCameraParams();
	validate();
}

void Settings::readApplicationParams(const cv::FileNode& node)
{
	node["Write_outputFileName"] >> outputFileName;
	node["RightInput"] >> rightInput;
	node["LeftInput"] >> leftInput;
}

void Settings::readCameraParams()
{
	cv::FileStorage fs(outputFileName, cv::FileStorage::READ);
	fs["Camera_Matrix"] >> cameraMatrix;
	fs["Distortion_Coefficients"] >> distCoeffs;
	fs.release();
}

void Settings::validate()
{
	goodInput = true;

	if (rightInput[0] >= '0' && rightInput[0] < '9' && leftInput[0] >= '0' && leftInput[0] < '9')
	{
		// An integer identies a camera device.
		inputType = CAMERA;
	}
	else if (rightInput.empty() || leftInput.empty())
	{
		// An empty input is invalid.
		inputType = INVALID;
	}
	else
	{
		// Must be a video file. We could test for a string at least, or even a valid path to be sure.
		inputType = VIDEO_FILE;
	}

	if (inputType == INVALID)
	{
		std::cerr << " Inexistent input: " << rightInput;
		std::cerr << " Left input: " << leftInput;
		goodInput = false;
	}

	flag = 0;
}

static bool readStringList(const std::string& filename, cv::vector<cv::string>& l)
{
	l.clear();
	cv::FileStorage fs(filename, cv::FileStorage::READ);
	if (!fs.isOpened())
		return false;
	cv::FileNode n = fs.getFirstTopLevelNode();
	if (n.type() != cv::FileNode::SEQ)
		return false;
	cv::FileNodeIterator it = n.begin(), it_end = n.end();
	for (; it != it_end; ++it)
		l.push_back((std::string)*it);
	return true;
}

cv::Mat Settings::getCameraMatrix()
{
	return cameraMatrix;
}

cv::Mat Settings::getDistCoeffs()
{
	return distCoeffs;
}


