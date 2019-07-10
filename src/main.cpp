#include <cstdint>
#include "opencv2/opencv.hpp"
#include <cstdio>
#include "Synth.h"

using namespace std;
using namespace cv;

const int fps = 60;

int main(int argc, char const *argv[])
{
    Synth s;


    Mat frame;

    VideoCapture vid(0);

    if (!vid.isOpened())
    {
        return -1;
    }
    
    while (vid.read(frame))
    {
        imshow("Webcam", frame);
        if(waitKey(1000 / fps) != 255)
        {
            break;
       }
    }
    
    return 0;
}