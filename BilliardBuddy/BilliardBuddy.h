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
#include "TextAugmentor.h"

class BilliardBuddy
{
private:
	static bool processFrame(bool& preprocess, CameraInterface& cameraInterface, PreProcessor& preProcessor, PoolTableDetector& poolTableDetector, TextAugmentor textAugmentor);
	static bool pollKeyboard(bool& preprocess);

public:
	static void help();
	static void process(Settings& settings);
};