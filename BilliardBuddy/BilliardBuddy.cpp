#include "BilliardBuddy.h"

int main(int argc, char* argv[])
{
	BilliardBuddy::help();

	// Read config settings.
	const string inputSettingsFile = argc > 1 ? argv[1] : "config.xml";
	FileStorage fs(inputSettingsFile, FileStorage::READ); 
	if (!fs.isOpened())
	{
		std::cout << "Could not open the configuration file: \"" << inputSettingsFile << "\"" << std::endl;
		return -1;
	}
	Settings settings;
	fs["Settings"] >> settings;
	fs.release();

	// Check validity of inputs before training begins.
	if (!settings.goodInput)
	{
		std::cout << "Invalid input detected. Application stopping. " << std::endl;
		return -1;
	}

	// MAIN LOGIC
	BilliardBuddy::process(settings);

	return 0;
}

void BilliardBuddy::process(Settings& settings) {
	bool showUndistorted = true;

	// Initialize CameraInterface
	CameraInterface cameraInterface = CameraInterface(settings);

	// Initialize PreProcessor
	Mat cameraMatrix, distCoeffs;
	cameraMatrix = settings.getCameraMatrix();
	distCoeffs = settings.getDistCoeffs();
	PreProcessor preProcessor = PreProcessor(cameraMatrix, distCoeffs);

	// Initialize Feature Detector(s)
	PoolTableDetector poolTableDetector = PoolTableDetector();

	// Initialize Visual Augmentor(s)
	TextAugmentor textAugmentor = TextAugmentor();

	clock_t prevTimestamp = 0;
	const char ESC_KEY = 27;

	// Processing Loop
	for (int i = 0;; ++i)
	{
		// Fetch feed from camera interface.
		Mat leftFrame, rightFrame;
		cameraInterface.getRightFrame(rightFrame);
		cameraInterface.getLeftFrame(leftFrame);

		// Pre-process feed.
		if (showUndistorted)
		{
			preProcessor.preProcess(leftFrame);
			preProcessor.preProcess(rightFrame);
		}

		// Detect features.
		poolTableDetector.detect(rightFrame);

		// Visually augment (with text as an example).
		textAugmentor.augment(rightFrame);

		// Display feed to user.
		imshow("Left", leftFrame);
		imshow("Right", rightFrame);

		// Check for keyboard inputs.
		char key = (char)waitKey(20);

		if (key == ESC_KEY)
			break;

		if (key == 'u')
			showUndistorted = !showUndistorted;
	}
}

void BilliardBuddy::help()
{
	std::cout << "Welcome to the BilliardBuddy trainer!" << std::endl
		<< "Usage: BilliardBuddy.exe <configpath.xml>" << std::endl
		<< "Near the sample file you'll find the configuration file, which has detailed help of "
		"how to edit it.  It may be any OpenCV supported file format XML/YAML." << std::endl;
}

void read(const FileNode& node, Settings& x, const Settings& default_value = Settings())
{
	// This is called when the ">>" operator is used between a FileNode and a Settings. 
	if (node.empty())
		x = default_value;
	else
		x.read(node);
}