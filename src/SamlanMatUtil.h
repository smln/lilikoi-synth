/**
 * Image processing functions for my app... to keep Main.cpp less cluttered
 */

#pragma once

#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"

#include <cstdint>
#include <cstdio>
#include <math.h>
#include <tuple>

#include "OpticalFlowMat.h"

using namespace cv;
using namespace std;

/**
 * Determine the dominant colors in an image, or if the image is too gray, white, or black to have significant color info, return that.
 * @param downsampledim The image to analyze, should be already downsampled for efficiency's sake
 * @return <1> max_hues (3 element vector containing 3 most dominant hues, empty vector if image color is not significant), 
 *              <2> is_mostly_black, <3> is_mostly_white, <4> saturation
 */
tuple<vector<int>, bool, bool, float> getDominantColorsInImage(Mat& downsampledim);


/**
 * If the camera is moving, this function should detect that and return some parameters about the camera's movement.
 * If the camera is not moving, but objects in the scene are, this function should determine how much movement is in the scene.
 * @param optical_flow 2 channel matrix containing the dense optical flow (x and y) between the images we're analyzing
 * @param scene_movement_threshold If the percentage of pixels in the image with significant movement is below this, we will consider it
 *                                  scene object movement rather than camera movement.
 * @param flow_sensitivity Higher numbers means smaller flow amounts are considered significant movement
 * @return <1> scene_movement_percentage (range 0-1, or -1 if camera movement detected), <2> camera_moved_down, <3> camera_moved_up,
 *            <4> camera_moved_left, <5> camera_moved_right, <6> camera_spinning
 */
tuple<float, bool, bool, bool, bool, bool> getSceneMovementFromFlow
        (OpticalFlowMat& optical_flow, float scene_movement_threshold = 0.55, float flow_sensitivity = 20);

/**
 * Gamma correction
 */
void correctGamma( Mat& img, double gamma );

/**
 * downsample an image to between 64 and 128 px on its largest dimension
 * if it's already smaller than 128 px then do nothing
 * @param from The input image
 * @param to The downsampled image
 * @TODO inspect for possible edge case errors
*/
void downSampleImage(Mat& from, Mat& to);

/**
 * Get the dominant hue in a photo
 * @param hue_image Should be hue matrix
 * @param sat_threshold If pixels in the image have saturation < sat_threshold they will be pruned
 * @param val_threshold If pixels in the image have value < val_threshold they will be pruned
 * @param huebins should be > 3
 * @return Dominant 3 hues in the image as ints in a vector range 0-180
 */
vector<int> getDominantHues(Mat& hue_image, const int sat_threshold, const int val_threshold, int huebins = 15);

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