#include "Settings.h"

void Settings::read(const FileNode& node)                          //Read serialization for this class
{
	node["Write_outputFileName"] >> outputFileName;
	node["Show_UndistortedImage"] >> showUndistorsed;
	node["RightInput"] >> rightInput;
	node["LeftInput"] >> leftInput;
	interprate();
}

void Settings::interprate()
{
	goodInput = true;

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
}

Mat Settings::nextImage(VideoCapture& captureSource)
{
	Mat result;
	if (captureSource.isOpened())
	{
		Mat view0;
		captureSource >> view0;
		view0.copyTo(result);
	}

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

Settings::~Settings()
{
}
