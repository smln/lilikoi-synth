#include "OpticalFlowMat.h"

float OpticalFlowMat::getFastMovementSize()
{
    return std::max(this->cols, this->rows) / 10;
}