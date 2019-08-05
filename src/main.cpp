#include <cstdint>
#include <cstdio>
#include <math.h>

#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"
#include <opencv2/imgproc.hpp>

#include "Synth.h"
#include "SamlanMatUtil.h"

using namespace std;
using namespace cv;

const int fps = 60;

const int LOW_CANNY_THRESHOLD = 30; //sensitivity of edge detector, can be like 10-100, around 30 or 40 was seeming to work best
const int LOW_SAT_THRESHOLD = 50; //threshold of image saturation (GRAY) at which hue detection will stop/start (S and V can range 0-255)
const int LOW_VAL_THRESHOLD = 80; //threshold of image value(BLACK) at which hue detection will stop/start
const int HI_VAL_THRESHOLD = 190; //threshold of image value (WHITE when sat. is low) at which hue detection will stop/start

//gonna reuse these data structures in each loop
vector<vector<Point>> contours;
vector<Mat> hsv_planes;
Mat camimage, edgy, downsampledim, output, edgydownsamp, currentsmallgrayim, hsvsmall,
    val_image, hue_image, sat_image, lastsmallgrayim;

vector<vector<Point>> nocolor_rect(1, vector<Point>(4));
vector<vector<Point>> hue_rect1(1, vector<Point>(4)); //dominant hue
vector<vector<Point>> hue_rect2(1, vector<Point>(4)); //2nd most dominant hue
vector<vector<Point>> hue_rect3(1, vector<Point>(4)); //3rd most dominant hue


/**
 * initialize the bounding areas for drawing colored boxes on the gui
 */
void makeColorBoxes(Mat& camimage)
{
    int xleft = (1 * camimage.cols) / 8;
    int ytop = 3 * camimage.rows / 4;
    int xwidth = 3 *camimage.cols / 4;
    int yheight = camimage.rows / 4;

    nocolor_rect[0][0] = Point(xleft , ytop);
    nocolor_rect[0][1] = Point(xleft + xwidth , ytop);
    nocolor_rect[0][2] = Point(xleft + xwidth, ytop + yheight );
    nocolor_rect[0][3] = Point(xleft, ytop + yheight);

    hue_rect1[0][0] = Point(xleft, ytop);
    hue_rect1[0][1] = Point(xleft + (xwidth / 3), ytop);
    hue_rect1[0][2] = Point(xleft + (xwidth / 3), ytop + yheight);
    hue_rect1[0][3] = Point(xleft, ytop + yheight);

    hue_rect2[0][0] = Point(xleft + (xwidth / 3), ytop);
    hue_rect2[0][1] = Point(xleft + (2 * xwidth / 3), ytop);
    hue_rect2[0][2] = Point(xleft + (2 * xwidth / 3), ytop + yheight);
    hue_rect2[0][3] = Point(xleft+ (xwidth / 3), ytop + yheight);

    hue_rect3[0][0] = Point(xleft + (2 * xwidth / 3), ytop);
    hue_rect3[0][1] = Point(xleft + xwidth, ytop);
    hue_rect3[0][2] = Point(xleft + xwidth, ytop + yheight);
    hue_rect3[0][3] = Point(xleft + (2 * xwidth / 3), ytop + yheight);
}

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

    //make the gui
    string windowname = "Reverse Visualizer";
    namedWindow( windowname, WINDOW_AUTOSIZE );

    
    bool firstloop = true;

    Mat bigcurrentgray, lastbiggray;

    //main loop for doing image processing
    while (vid.read(camimage))
    {   
        if(firstloop)
        {
            makeColorBoxes(camimage);
        }

        //PREPARE MATRICES
        //@@@test
        cvtColor(camimage, bigcurrentgray, COLOR_BGR2GRAY);

        output = camimage.clone();
        
        downSampleImage(camimage, downsampledim); //make it small for lower cpu load

        //separate out different hsv channels
        cvtColor(downsampledim, hsvsmall, COLOR_BGR2HSV );
        split(hsvsmall, hsv_planes);
        hue_image = hsv_planes[0]; //CV_8U1 type
        sat_image = hsv_planes[1];
        val_image = hsv_planes[2];

        edgy = downsampledim.clone();
        cvtColor(edgy, currentsmallgrayim, COLOR_BGR2GRAY);


        //###############################################################
        //TASK 1: Find and send number of contours
        
        //do canny edge detection
        GaussianBlur(currentsmallgrayim, edgy, Size(7,7), 1.5, 1.5);
        Canny(edgy, edgy, LOW_CANNY_THRESHOLD, LOW_CANNY_THRESHOLD * 2.5 /* the ratio */);

        //find contours from edge detected image
        findContours(edgy, contours, RETR_LIST, CHAIN_APPROX_SIMPLE);

        //send number of contours to pd
        int contoursend = contours.size();
        Synth::pd.sendFloat("contoursfromcpp", contoursend);
        // cout << "num countours: " << contoursend << endl;

        //this is for displaying the detected edges for neat effect
        cvtColor(edgy, edgy, COLOR_GRAY2BGR);
        resize(edgy, edgy, Size(camimage.cols, camimage.rows), 0, 0, INTER_NEAREST);

        addWeighted(camimage, 1, edgy, 0.6, 0, output);
        //################################################################



        //################################################################
        //TASK 2: Detect and send hue if significant, if not send "gray" "light" or "dark" message

        // cout << "Image value: " << mean(val_image)[0] << endl;
        // cout << "Image saturation: " << mean(sat_image)[0] << endl;

        float valuemean = mean(val_image)[0];
        float satmean = mean(sat_image)[0];

        if(valuemean < LOW_VAL_THRESHOLD)
        {
            //image is black
            Synth::pd.sendBang("imagedarkfromcpp");
            fillPoly(output, nocolor_rect, Scalar(0, 0, 0) );
        }
        else if(satmean < LOW_SAT_THRESHOLD)
        {
            if(valuemean > HI_VAL_THRESHOLD)
            {
                //image is white
                Synth::pd.sendBang("imagewhitefromcpp");
                fillPoly(output, nocolor_rect, Scalar(255, 255, 255) );
            }
            else
            {
                //image is gray
                Synth::pd.sendBang("imagegrayfromcpp");
                fillPoly(output, nocolor_rect, Scalar(80, 80, 80) );
            }
            
        }
        else
        {
            //color is significant so we send 3 dominant hues
            vector<int> maxhues = getDominantHues(hue_image);

            //we need to convert the dominant hues to bgr to display them on the gui
            vector<Vec3b> maxhueshsv(3);
            vector<Vec3b> maxhuesbgr(3);
            maxhueshsv[0] = Vec3b(maxhues[0], 255, 255);
            maxhueshsv[1] = Vec3b(maxhues[1], 255, 255);
            maxhueshsv[2] = Vec3b(maxhues[2], 255, 255);
            cvtColor(maxhueshsv, maxhuesbgr, COLOR_HSV2BGR);

            fillPoly(output, hue_rect1, maxhuesbgr[0] );
            fillPoly(output, hue_rect2, maxhuesbgr[1] );
            fillPoly(output, hue_rect3, maxhuesbgr[2] );

            Synth::pd.sendFloat("huemaxfromcpp", maxhues[0]);
            Synth::pd.sendFloat("huesecondmaxfromcpp", maxhues[1]);
            Synth::pd.sendFloat("huethirdmaxfromcpp", maxhues[2]);
        }
        //#################################################################




        //#################################################################
        //TASK 3: Motion detection

        Mat bgr = downsampledim.clone();
        if(!firstloop)
        {
            Mat flow;
            calcOpticalFlowFarneback(lastsmallgrayim, currentsmallgrayim, flow, 0.5, 4, 17, 3, 5, 1.1);
            // visualization
            Mat flow_parts[2];
            split(flow, flow_parts);
            Mat magnitude, angle, magn_norm;
            cartToPolar(flow_parts[0], flow_parts[1], magnitude, angle, true);
            normalize(magnitude, magn_norm, 0.0f, 1.0f, NORM_MINMAX);
            angle *= ((1.f / 360.f) * (180.f / 255.f));
            //build hsv image
            Mat _hsv[3], hsv, hsv8;
            _hsv[0] = angle;
            _hsv[1] = Mat::ones(angle.size(), CV_32F);
            _hsv[2] = magn_norm;
            merge(_hsv, 3, hsv);
            hsv.convertTo(hsv8, CV_8U, 255.0);
            cvtColor(hsv8, bgr, COLOR_HSV2BGR);
            
            // add(bgr, downsampledim, bgr);
            //This optical flow stuff comes from https://docs.opencv.org/3.4/d4/dee/tutorial_optical_flow.html
        }

        //##################################################################




        //Some GUI stuff
        addWeighted(output, 1, edgy, 0.6, 0, output);
        resize(output, output, Size(), 1.5, 1.5, INTER_NEAREST);
        if(!firstloop) 
        {
            resize(bgr, bgr, Size(), 10, 10, INTER_NEAREST);
            imshow(windowname, bgr);
        }
        
        lastsmallgrayim = currentsmallgrayim.clone();
        //@@@test
        lastbiggray = bigcurrentgray.clone();

        if(firstloop)
        {
            firstloop = false;
            Synth::pd.sendBang("cppinitialized"); // might need this in pd?
        }
        
        if(waitKey(1000 / fps) != 255) //exit on key press
        {
            break;
        }
    }
    
    return 0;
}