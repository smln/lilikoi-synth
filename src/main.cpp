#include <cstdint>
#include <cstdio>
#include <math.h>
#include <tuple>
#include <chrono>
#include <thread>

#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"
#include <opencv2/imgproc.hpp>

#include "Synth.h"
#include "SamlanMatUtil.h"
#include "FancyDisplayCanvas.h"

using namespace std;
using namespace cv;

const int fps = 24; //framerate

const int LOW_CANNY_THRESHOLD = 30; //sensitivity of edge detector, can be like 10-100, around 30 or 40 was seeming to work best

//gonna reuse these data structures in each loop
Mat downsampledim; //Smaller version of current camera image, for efficiency's sake. Store in BGR
Mat camimage; //most recent image grab from the camera
Mat currentsmallgrayim; //downsampled grayscale version of most recent image grab from camera
Mat lastsmallgrayim; //downsampled grayscale version of previous camera frame



int main(int argc, char const *argv[])
{
    //Start PD
    Synth s;
    s.playAudio();

    //Open the webcam
    VideoCapture vid(0);
    // VideoCapture vid("u.webm"); //for opening a video file

    if (!vid.isOpened())
    {
        cerr << "could not open video. Exiting." << endl;
        return -1;
    }

    //read the first frame and wait so when the loop starts it'll grab the next frame
    if(!vid.read(camimage)) return -2;
    using namespace std::this_thread;     // sleep_for, sleep_until
    using namespace std::chrono; // ns, us, ms, s, h, etc.
    sleep_for(milliseconds(1000/fps));

    //store this frame as the previous image for optical flow calculation
    downSampleImage(camimage, downsampledim);
    cvtColor(downsampledim, lastsmallgrayim, COLOR_BGR2GRAY);
    
    FancyDisplayCanvas canvas(camimage, "Lilikoi");

    bool firstloop = true;

    //main loop for doing image processing
    while (vid.read(camimage))
    {   
        canvas.clearToNewImage(camimage);

        //PREPARE MATRICES
        downSampleImage(camimage, downsampledim); //make it small for lower cpu load
        cvtColor(downsampledim, currentsmallgrayim, COLOR_BGR2GRAY);


        //###############################################################
        //TASK 1: Find and send number of contours

        //do canny edge detection
        Mat edges_small;
        GaussianBlur(currentsmallgrayim, edges_small, Size(7,7), 1.5, 1.5);
        Canny(edges_small, edges_small, LOW_CANNY_THRESHOLD, LOW_CANNY_THRESHOLD * 2.5 /* the ratio */);

        //find contours from edge detected image
        vector<vector<Point>> contours;
        findContours(edges_small, contours, RETR_LIST, CHAIN_APPROX_SIMPLE);

        //send number of contours to pd
        Synth::pd.sendFloat("contoursfromcpp", contours.size());
        // cout << "num countours: " << contours.size() << endl;

        //this is for displaying the detected edges for neat effect
        cvtColor(edges_small, edges_small, COLOR_GRAY2BGR);
        canvas.addSmall(edges_small, 0.6);
        //################################################################


        //################################################################
        //TASK 2: Detect and send hue if significant, if not send "gray" "light" or "dark" message

        //yay for tuples
        auto [max_hues, is_mostly_black, is_mostly_white, saturation] = getDominantColorsInImage(downsampledim);

        Synth::pd.sendFloat("saturationfromcpp", saturation);
        cout << saturation << endl;
        
        if(max_hues.empty()) //no significant color in image
        {
            if(is_mostly_black)
            {
                Synth::pd.sendBang("imagedarkfromcpp");
                canvas.makeBlackImageRectangle();
            }
            else if(is_mostly_white)
            {
                Synth::pd.sendBang("imagewhitefromcpp");
                canvas.makeWhiteImageRectangle();
            }
        }
        else //color in image is significant
        {
            canvas.makeColoredImageRectangle(max_hues);

            //send three dominant hues to PD, values range 0-180

            Synth::pd.sendFloat("huethirdmaxfromcpp", max_hues[2]);
            Synth::pd.sendFloat("huesecondmaxfromcpp", max_hues[1]);
            Synth::pd.sendFloat("huemaxfromcpp", max_hues[0]);
        }
        //#################################################################



        //#################################################################
        //TASK 3: Motion detection

        OpticalFlowMat optical_flow;
        // cout << optical_flow << endl;

        //calculate optical flow
        calcOpticalFlowFarneback(lastsmallgrayim, currentsmallgrayim, optical_flow, 0.5, 1, 18, 3, 5, 1.1, 0);

        // @@@TODO tweak flow_sensitivity and scene_movement_threshold for better motion detection
        auto[scene_movement_percentage, camera_moved_down, camera_moved_up, 
                camera_moved_left, camera_moved_right, camera_spinning] = getSceneMovementFromFlow(optical_flow);

        if(scene_movement_percentage >= 0) { Synth::pd.sendFloat("sceneobjectmovementfromcpp", scene_movement_percentage); }
        if(camera_moved_down) { Synth::pd.sendBang("cameramovedownfromcpp"); }
        if(camera_moved_up) { Synth::pd.sendBang("cameramoveupfromcpp"); }
        if(camera_moved_left) { Synth::pd.sendBang("cameramoveleftfromcpp"); }
        if(camera_moved_right) { Synth::pd.sendBang("cameramoverightfromcpp"); }
        if(camera_spinning) { Synth::pd.sendBang("cameraspinningfromcpp"); }

        canvas.addOpticalFlowVisual(optical_flow);
        
        //This optical flow stuff comes from https://docs.opencv.org/3.4/d4/dee/tutorial_optical_flow.html
        //##################################################################




        //Some GUI stuff
        canvas.draw();
        
        lastsmallgrayim = currentsmallgrayim.clone();

        if(firstloop)
        {
            Synth::pd.sendBang("cppinitialized"); // might need this in pd? send when all params have been initialized
            firstloop = false;
        }
        
        if(waitKey(1000 / fps) != 255) //exit on key press
        {
            break;
        }

        //@@@for testing
        Synth::pd.receiveMessages();
    }
    
    return 0;
}