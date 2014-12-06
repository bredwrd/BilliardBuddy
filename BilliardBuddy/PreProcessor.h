#pragma once

#include <opencv2/core/core.hpp>
#include <opencv2/ocl/ocl.hpp>

using namespace cv;
using namespace ocl;

class PreProcessor
{
private:
	void oclUndistort(Mat& frame);
	void cpuUndistort(Mat& frame);
	void transposeImage(Mat& frame);

	Size imageSize;
	Mat cameraMatrix;
	Mat distCoeffs;

public:
	PreProcessor(Mat& cameraMatrix, Mat& distCoeffs);
	~PreProcessor();

	void preProcess(Mat& frame);


};

