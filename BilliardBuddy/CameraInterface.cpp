#include "CameraInterface.h"

CameraInterface::CameraInterface(Settings& settings)
{
	initializeVideoSources(settings);
}

CameraInterface::~CameraInterface()
{

}

void CameraInterface::grabFrames()
{
		leftVideoCapture.grab();
		rightVideoCapture.grab();
}

void CameraInterface::retrieveFrames(cv::Mat& leftFrame, cv::Mat& rightFrame)
{
	leftVideoCapture.retrieve(leftFrame);
	rightVideoCapture.retrieve(rightFrame);
}

void CameraInterface::getFrames(cv::Mat& leftFrame, cv::Mat& rightFrame)
{
	if (leftVideoCapture.isOpened() && rightVideoCapture.isOpened())
	{
		grabFrames();
		retrieveFrames(leftFrame, rightFrame);
	}
}


void CameraInterface::initializeVideoSources(Settings& settings) {
	if (settings.rightInput[0] >= '0' && settings.leftInput[0] <= '9')
	{
		std::stringstream ssRight(settings.rightInput);
		std::stringstream ssLeft(settings.leftInput);
		ssRight >> settings.rightCameraID;
		ssLeft >> settings.leftCameraID;
		settings.inputType = settings.CAMERA;
	}
	else
	{
		if (false)
		{
			settings.inputType = settings.IMAGE_LIST;
		}
		else
			settings.inputType = settings.VIDEO_FILE;
	}
	if (settings.inputType == settings.CAMERA) {
		rightVideoCapture.open(settings.rightCameraID);
		leftVideoCapture.open(settings.leftCameraID);
	}
	else if (settings.inputType == settings.VIDEO_FILE) {
		rightVideoCapture.open(settings.rightInput);
		leftVideoCapture.open(settings.leftInput);
	}
}
