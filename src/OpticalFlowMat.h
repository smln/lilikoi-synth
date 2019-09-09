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
     * Variadic template constructor. Passes arguments on to Mat constructor.
     * Template can't go in .cpp files I guess.
     */
    template<typename ...Args> OpticalFlowMat(Args&&... args) 
    :Mat(std::forward<Args>(args)...) 
    {
        fast_movement_size = std::max(this->cols, this->rows) / 10;
    }

    /**
     * @return Approximate flow amount in pixels we're expecting for a fast movement
     */
    float getFastMovementSize();

    private:
    float fast_movement_size;
};