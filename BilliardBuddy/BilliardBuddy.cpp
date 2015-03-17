#include "BilliardBuddy.h"
#include <iostream>
using namespace std;

int BilliardBuddy::frameIterator = 0;
cv::vector<Vec2i> BilliardBuddy::cueCoords = cv::vector<Vec2i>(0, 0);
// Create blank cueMask;
cv::Mat BilliardBuddy::cueMask = cv::Mat(640, 480, CV_8UC3, cv::Scalar(0, 0, 0));

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
	bool preprocess = true;

	// Initialize CameraInterface
	CameraInterface cameraInterface = CameraInterface(settings);

	// Initialize PreProcessor
	Mat cameraMatrix, distCoeffs;
	cameraMatrix = settings.getCameraMatrix();
	distCoeffs = settings.getDistCoeffs();
	PreProcessor preProcessor = PreProcessor(cameraMatrix, distCoeffs);

	// Initialize Feature Detector(s)
	PoolTableDetector poolTableDetector = PoolTableDetector();
	CueDetector cueDetector = CueDetector();

	//Initialize Physics Model Calculator
	PhysicsModel physicsModel = PhysicsModel();

	// Initialize Visual Augmentors
	TrajectoryAugmentor trajectoryAugmentor = TrajectoryAugmentor();
	TextAugmentor textAugmentor = TextAugmentor();
	CueAugmentor cueAugmentor = CueAugmentor();

	// Initialize HMD interface.
	HMDInterface hmdInterface = HMDInterface();

	// Processing Loop
	bool result;
	do {
		result = processFrame(preprocess, cameraInterface, preProcessor, poolTableDetector, cueDetector, physicsModel, textAugmentor, cueAugmentor, hmdInterface, trajectoryAugmentor);
		result = pollKeyboard(preprocess);
	} while (result == true);
}

bool BilliardBuddy::pollKeyboard(bool& preprocess)
{
	// Check for keyboard inputs.
	char key = (char)waitKey(1);
	const char ESC_KEY = 27;
	if (key == 'u')
		preprocess = !preprocess;
	if (key == ESC_KEY)
		return false;

	return true;
}

bool BilliardBuddy::processFrame(bool& preprocess, CameraInterface& cameraInterface, PreProcessor& preProcessor, PoolTableDetector& poolTableDetector, CueDetector& cueDetector, PhysicsModel& physicsModel, TextAugmentor& textAugmentor, CueAugmentor& cueAugmentor, HMDInterface& hmdInterface, TrajectoryAugmentor& trajectoryAugmentor) {
	// Fetch feed from camera interface.
	Mat leftFrame, rightFrame;
	cameraInterface.getFrames(leftFrame, rightFrame);

	// Pre-process feed.
	if (preprocess)
	{
		preProcessor.preProcess(leftFrame);
		preProcessor.preProcess(rightFrame);
	}

	//Detect Cue
	if (frameIterator == 1 || frameIterator == 0)
	{
		cueCoords = cueDetector.detect(rightFrame, frameIterator);
		cueDetector.getCueMask(cueMask);
	}

	// Detect features.
	poolTableDetector.setCueMask(cueMask);
	cv::vector<pocket> pocketPoints = poolTableDetector.detectTable(rightFrame, frameIterator);
	
	// Find target pocket location
	cv::vector<cv::Vec2i> whiteBall = poolTableDetector.getCueBallCoords();
	BallDetector ballDetector = BallDetector();

	cv::Point2f targetPocket = getTargetPocket(pocketPoints, float(360), float(0));
	cv::vector<cv::Vec2i> ballPosition = ballDetector.detectByTargetPocket(rightFrame, frameIterator, targetPocket);
	//cout << ballPosition[0] << endl;
	//Detect other balls
	//TODO?? Brian?
	cv::vector<Vec2f> balls(1);
	balls[0] = ballPosition[0]; //temp
	//balls[1] = { 200, 170 };
	//balls[2] = { 150, 150 };
	cv::vector<Path> pathVector;

	if (poolTableDetector.getDefPerspective() == true){
		//Calculate Physics Model
		pathVector = physicsModel.calculate(rightFrame, pocketPoints, cueCoords, whiteBall, balls);
	}
	else{
		Path tempPath;
		tempPath.endPoint = 0;
		tempPath.startPoint = 0;
		pathVector = { tempPath };
	}

	//Visually Augment
	trajectoryAugmentor.augment(rightFrame, pathVector);
	imshow("3D", rightFrame);
	textAugmentor.augment(rightFrame);
	//cueAugmentor.augment(rightFrame, cueCoords);
	hmdInterface.drawToHMD(leftFrame, rightFrame);

	if (frameIterator == 7)
	{
		frameIterator = 1;
	}
	else
	{
		frameIterator++;
	}

	// check for errors and return false at some point
	return true;
}

int BilliardBuddy::getFrameIterator()
{
	return frameIterator;
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

//Returns target pocket 3D coordinates given specified 2D coordinates (See physics model or point locator for 2D coordinates)
cv::Point2f BilliardBuddy::getTargetPocket(cv::vector<pocket> pockets, float xDestination, float yDestination){
	//Initializes targetPocket default
	cv::Point2f targetPocket;
	targetPocket.x = float(0);
	targetPocket.y = float(0);

	//Sets epsilon value for comparing floats
	float epsilon = float(0.05);

	//Loops through all pockets to check if they have destination points specified
	for (int i = 0; i < pockets.size(); i++){
		if (abs(pockets[i].xLocation - xDestination) < epsilon && abs(pockets[i].yLocation - yDestination) < epsilon){
			targetPocket.x = pockets[i].pocketPoints.pt.x;
			targetPocket.y = pockets[i].pocketPoints.pt.y;
		}
	}

	return targetPocket;
}