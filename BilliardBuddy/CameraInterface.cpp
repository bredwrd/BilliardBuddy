#include "CameraInterface.h"

CameraInterface::CameraInterface(Settings& settings)
{
	initializeVideoSources(settings);
}

CameraInterface::~CameraInterface()
{

}

void CameraInterface::getLeftFrame(cv::Mat& frame) {
	getFrame(leftVideoCapture, frame);
}

void CameraInterface::getRightFrame(cv::Mat& frame) {
	getFrame(rightVideoCapture, frame);
}

void CameraInterface::getFrame(cv::VideoCapture& captureSource, cv::Mat& frame)
{
	if (captureSource.isOpened())
	{
		captureSource >> frame;
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
