#pragma once

#include "opencv2/opencv.hpp"

using namespace cv;

/**
 * Try to make things a little more object oriented here.
 * This is basically a Mat representing an optical flow field.
 */
class OpticalFlowMat : public cv::Mat
{
    public:
    /**
     * @return Approximate flow amount in pixels we're expecting for a fast movement
     */
    float getFastMovementSize();
};