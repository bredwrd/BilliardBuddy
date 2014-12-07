#include "PreProcessor.h"

PreProcessor::PreProcessor(Mat& cameraMatrix, Mat& distCoeffs) : cameraMatrix(cameraMatrix), distCoeffs(distCoeffs)
{
}

PreProcessor::~PreProcessor()
{
}

void PreProcessor::preProcess(Mat& frame)
{
	oclUndistort(frame);
	//cpuUndistort(frame);
	transposeImage(frame);
}

void PreProcessor::transposeImage(Mat& frame)
{
	// rotate image 90 deg. clockwise to compensate for portrait camera orientation
	transpose(frame, frame);
	flip(frame, frame, 1);
}

void PreProcessor::oclUndistort(Mat& frame)
{
	oclMat gpu_temp, gpu_view, gpu_map1, gpu_map2;

	Mat tempFrame = frame.clone();

	Mat map1 = cv::Mat(), map2 = Mat();
	initUndistortRectifyMap(cameraMatrix, distCoeffs, cv::noArray(), cameraMatrix, frame.size(), CV_32FC1, map1, map2);

	// Load matricies as ocl-compatible matricies.
	gpu_temp.upload(tempFrame);
	gpu_view.upload(frame);
	gpu_map1.upload(map1);
	gpu_map2.upload(map2);

	ocl::remap(gpu_temp, gpu_view, gpu_map1, gpu_map2, INTER_NEAREST, BORDER_CONSTANT, 0);

	// Unload (only remap-prerequisite) matricies as CPU-formatted OpenCV matricies
	gpu_view.download(frame);
}

void PreProcessor::cpuUndistort(Mat& view)
{
	Mat temp = view.clone();
	undistort(temp, view, cameraMatrix, distCoeffs);
}