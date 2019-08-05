/**
 * Image processing functions for my app... to keep Main.cpp less cluttered
 */

#pragma once

#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"

#include <cstdint>
#include <cstdio>
#include <math.h>

using namespace cv;
using namespace std;

/**
 * Gamma correction
 */
void correctGamma( Mat& img, double gamma );

/**
 * downsample an image to between 128 and 256 px on its largest dimension
 * if it's already smaller than 256 px then do nothing
 * @param from The input image
 * @param to The downsampled image
 * @TODO inspect for possible edge case errors
*/
void downSampleImage(Mat& from, Mat& to);

/**
 * Get the dominant hue in a photo
 * @param hue_image Should be hue matrix
 * @param huebins should be > 3
 * @return Dominant 3 hues in the image as ints in a vector range 0-180
 */
vector<int> getDominantHues(Mat hue_image, int huebins = 15);

/**
 * Get the edginess of an image
 * @return float 0-1 which is a percentage, 0 being no edges, 1 being all edges
 */
float getEdginess(Mat grayinput);

/**
 * from https://codeyarns.com/2015/08/27/depth-and-type-of-matrix-in-opencv/
 */
std::string GetMatType(const cv::Mat& mat);

/**
 * from https://codeyarns.com/2015/08/27/depth-and-type-of-matrix-in-opencv/
 */
std::string GetMatDepth(const cv::Mat& mat);