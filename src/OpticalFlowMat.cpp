#include "OpticalFlowMat.h"

// template<typename ...Args> OpticalFlowMat::OpticalFlowMat(Args&&... args) 
//     :Mat(std::forward<Args>(args)...) 
// {
//     fast_movement_size = std::max(this->cols, this->rows) / 10;
// }

float OpticalFlowMat::getFastMovementSize()
{
    return 10; //@@@TEST
    // return fast_movement_size;
}