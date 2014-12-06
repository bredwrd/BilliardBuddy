#include "TextAugmentor.h"

TextAugmentor::TextAugmentor()
{
}

TextAugmentor::~TextAugmentor()
{
}

void TextAugmentor::textAugment(cv::Mat& frame, std::string msg)
{
	// Define colours for text.
	const cv::Scalar RED(0, 0, 255), GREEN(0, 255, 0);

	// Add text to frame.
	int baseLine = 0;
	cv::Size textSize = cv::getTextSize(msg, 1, 1, 1, &baseLine);
	cv::Point textOrigin(frame.cols - 2 * textSize.width - 10, frame.rows - 2 * baseLine - 10);

	putText(frame, msg, textOrigin, 1, 1, RED);
}

void TextAugmentor::augment(cv::Mat& frame)
{
	// simple example of an augmentation
	textAugment(frame, "Message goes here.");
}
