#pragma once

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
#include "CameraInterface.h"
#include "PreProcessor.h"
#include "PoolTableDetector.h"
#include "CueDetector.h"
#include "CueAugmentor.h"
#include "TextAugmentor.h"
#include "PhysicsModel.h"
#include "HMDInterface.h"
#include "TrajectoryAugmentor.h"
#include "BallDetector.h"
#include "CueBallDetector.h"

class BilliardBuddy
{
private:
	static bool processFrame(bool& preprocess, CameraInterface& cameraInterface, PreProcessor& preProcessor, PoolTableDetector& poolTableDetector, CueDetector& cueDetector, PhysicsModel& physicsModel, TextAugmentor& textAugmentor, CueAugmentor& cueAugmentor, HMDInterface& hmdIntefrace, TrajectoryAugmentor& trajectoryAugmentor);
	static bool pollKeyboard(bool& preprocess);
	static int frameIterator;
	static cv::vector<Vec2i> cueCoords;
	static cv::Point2f getTargetPocket(cv::vector<pocket> pockets, float xDestination, float yDestination);
	static cv::Mat cueMask;

public:
	static void help();
	static void process(Settings& settings);
	static int getFrameIterator();
};