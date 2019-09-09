#pragma once

#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"

#include "OpticalFlowMat.h"

using namespace std;
using namespace cv;

/**
 *  Canvas to handle drawing some nice graphics on top of the webcam input
 */
class FancyDisplayCanvas
{
    public:
    FancyDisplayCanvas(Mat &camimage, string windowname);

    /**
     * Display the canvas
     */
    void draw();

    /**
     * Erases the current canvas and replaces it with a clone() of camimage
     */
    void clearToNewImage(Mat& camimage);

    /**
     * Add a full size image to the canvas
     * @param alpha the transparency of the new image
     */
    void add(Mat& image, float alpha = 1.0);

    /**
     * Add a downsampled image to the canvas. All small images should be the same size.
     * When draw() is called they are automatically resized and added to the output.
     * @param alpha the transparency of the new image
     */
    void addSmall(Mat &image, float alpha = 1.0);

    /**
     * Make a black rectangle to signify the image is mostly dark
     */
    void makeBlackImageRectangle();
    /**
     * Make a white rectangle to signify the image is mostly light
     */
    void makeWhiteImageRectangle();
    /**
     * Make a gray rectangle to signify the image is mostly gray
     */
    void makeGrayImageRectangle();
    /**
     * Make three colored rectangles to signify dominant image colors
     * @param dominant_hues A 3-element vector of the dominant hues in the image (range 0-180)
     */
    void makeColoredImageRectangle(vector<int> &dominant_hues);

    /**
     * Add the visuals for an optical for field
     * @param optical_flow Two channel (x and y) optical flow matrix
     */
    void addOpticalFlowVisual(OpticalFlowMat& optical_flow);

    private:
    /**
     * initialize the bounding areas for drawing colored boxes on the gui
     */
    void initializeColorBoxes();
    vector<vector<Point>> nocolor_rect; //rectangle for displaying gray, white, or black when the image has no significant color
    vector<vector<Point>> hue_rect1; //rectangle for displaying dominant hue
    vector<vector<Point>> hue_rect2; //rectangle for displaying 2nd most dominant hue
    vector<vector<Point>> hue_rect3; //rectangle for displaying 3rd most dominant hue

    string window_name;
    /**
     * The canvas we'll be adding different parts of the graphics to
     */
    Mat canvas;

    /**
     * Downsampled canvas we add things to, to efficiently resize during draw()
     */
    Mat small_canvas;
};