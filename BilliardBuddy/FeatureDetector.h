#pragma once

#include <opencv2/core/core.hpp>

// Use billbud namespace to prevent ambiguity with cv::FeatureDetecto.
namespace billbud
{
	class FeatureDetector
	{
	public:
		FeatureDetector();
		virtual void detect(cv::Mat& frame) = 0;
		virtual ~FeatureDetector();
	};
}


