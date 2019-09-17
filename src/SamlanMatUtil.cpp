#include "SamlanMatUtil.h"

tuple<vector<int>, bool, bool, float> getDominantColorsInImage(Mat& downsampledim)
{
    vector<int> max_hues;
    bool is_mostly_black = false, is_mostly_white = false; //return values

    const int LOW_SAT_THRESHOLD = 50; //threshold of image saturation (GRAY) at which hue detection will stop/start (S and V can range 0-255)
    const int LOW_VAL_THRESHOLD = 80; //threshold of image value(BLACK) at which hue detection will stop/start
    const int HI_VAL_THRESHOLD = 190; //threshold of image value (WHITE when sat. is low) at which hue detection will stop/start

    //separate out different hsv channels
    Mat hsvsmall;
    Mat hsv_planes[3];
    cvtColor(downsampledim, hsvsmall, COLOR_BGR2HSV );
    split(hsvsmall, hsv_planes);
    Mat hue_image = hsv_planes[0]; //CV_8U1 type. Hues of current downsampled image
    Mat sat_image = hsv_planes[1]; //HSV Saturation of current downsampled image
    Mat val_image = hsv_planes[2]; //HSV Value of current downsampled image

    // cout << "Image value: " << mean(val_image)[0] << endl;
    // cout << "Image saturation: " << mean(sat_image)[0] << endl;

    float valuemean = mean(val_image)[0];
    float satmean = mean(sat_image)[0];

    if(valuemean < LOW_VAL_THRESHOLD)
    {
        //image is black
        is_mostly_black = true;
    }
    else if(valuemean > HI_VAL_THRESHOLD)
    {
        //image is white
        is_mostly_white = true;
    }

    max_hues = getDominantHues(hsvsmall, LOW_SAT_THRESHOLD, LOW_VAL_THRESHOLD, 14);

    return {max_hues, is_mostly_black, is_mostly_white, satmean};
}


tuple<float, bool, bool, bool, bool, bool> getSceneMovementFromFlow
        (OpticalFlowMat & optical_flow, float scene_movement_threshold, float flow_sensitivity)
{
    //we'll tuple these up at the end and return them
    float scene_movement_percentage = -1;
    bool camera_moved_down = false, camera_moved_up = false, camera_moved_left = false, camera_moved_right = false, camera_spinning = false;
    
    //generate a histogram for the optical flow
    Mat xflow_hist, yflow_hist, flow_hist; //one bin for negative values, one for near zero values, one for positive values
    int xchannels[] = {0}; //compute x flow histogram from 0th channel
    int ychannels[] = {1}; //compute y flow histogram from 1st channel
    const int flowhistbins[1] = {3}; 
     //one bin for negative values, one for near zero values, one for positive values
    float histrange[] = {-(std::numeric_limits<float>::max()), -optical_flow.getFastMovementSize()/flow_sensitivity, 
                            optical_flow.getFastMovementSize()/flow_sensitivity, std::numeric_limits<float>::max()};
    const float *histranges[] = { histrange }; //cpp syntax is weird
    calcHist(&optical_flow, 1, xchannels, Mat(), xflow_hist, 1, flowhistbins, histranges, false);
    calcHist(&optical_flow, 1, ychannels, Mat(), yflow_hist, 1, flowhistbins, histranges, false);

    //for readability
    float y_down_flow_amount = yflow_hist.at<float>(0, 0);
    float y_no_flow_amount = yflow_hist.at<float>(1, 0);
    float y_up_flow_amount = yflow_hist.at<float>(2, 0);
    float x_right_flow_amount = xflow_hist.at<float>(0, 0);
    float x_no_flow_amount = xflow_hist.at<float>(1, 0);
    float x_left_flow_amount = xflow_hist.at<float>(2, 0);

    float totalxmovement = x_left_flow_amount + x_right_flow_amount;
    float totalymovement = y_down_flow_amount + y_up_flow_amount;
    float totalpixels = totalxmovement + x_no_flow_amount; //total pixels for which flow was calculated (for one channel)
    float totalxmovementpercent = totalxmovement / totalpixels;
    float totalymovementpercent = totalymovement / totalpixels;
    if(totalxmovementpercent < scene_movement_threshold && totalymovementpercent < scene_movement_threshold) //camera is not moving but objects in scene might be
    {
        //this ranges from 0 to 1
        scene_movement_percentage = (totalxmovement + totalymovement) / (totalpixels*2);
        // cout << "scene movement: " << totalxmovementpercent + totalymovementpercent << endl;
    }
    else //camera is moving OR there's some chaotic thing going on in the image
    {
        //@@@TODO there's some stuff I can tweak in these equations to possibly make motion detection work better
        //detect vertical movement
        if(y_down_flow_amount > y_up_flow_amount*4 && y_down_flow_amount > y_no_flow_amount)
        {
            camera_moved_down = true;
            // cout << "camera moving down" << endl;
        }
        else if(y_up_flow_amount > y_down_flow_amount*4 && y_up_flow_amount > y_no_flow_amount)
        {
            camera_moved_up = true;
            // cout <<"camera moving up" << endl;
        }
        
        //detect horizontal movement
        if(x_left_flow_amount > x_right_flow_amount*4 && x_left_flow_amount > x_no_flow_amount)
        {
            camera_moved_left = true;
            // cout << "camera moving left" << endl;
        }
        else if(x_right_flow_amount > x_left_flow_amount*4 && x_right_flow_amount > x_no_flow_amount)
        {
            camera_moved_right = true;
            // cout <<"camera moving right" << endl;
        }

        //detect camera spin. Note that this might also denote weird chaotic things happening in the image.
        if( (std::abs(x_left_flow_amount - x_right_flow_amount) < totalpixels/3 && totalxmovement > x_no_flow_amount*2)
            || (std::abs(y_up_flow_amount - y_down_flow_amount) < totalpixels/3 && totalymovement > y_no_flow_amount*2) )
        {
            camera_spinning = true;
            // cout << "camera spinning" << endl;
        }
    }

    return {scene_movement_percentage, camera_moved_down, camera_moved_up, 
                    camera_moved_left, camera_moved_right, camera_spinning};

}



void correctGamma( Mat& img, double gamma )
{
    double inverse_gamma = 1.0 / gamma;
    Mat lut_matrix(1, 256, CV_8UC1 );
    uchar * ptr = lut_matrix.ptr();

    for( int i = 0; i < 256; i++ )
    {
        ptr[i] = (int)( pow( (double) i / 255.0, inverse_gamma ) * 255.0 );
    }
    
    LUT( img, lut_matrix, img );

}

//@TODO add parallelism, see https://docs.opencv.org/4.1.0/d7/dff/tutorial_how_to_use_OpenCV_parallel_for_.html
const static int MAX_DSAMP_SIZE = 128;
void downSampleImage(Mat& from, Mat& to)
{
    //take the log base 2 of whichever dimension (x or y) is longer.
    int largestdimension = from.rows > from.cols ? from.rows : from.cols;
    float downsamplingfactor = 1.0 / ((largestdimension / MAX_DSAMP_SIZE) + 1);
    resize(from, to, Size(), downsamplingfactor, downsamplingfactor, INTER_NEAREST);
}



vector<int> getDominantHues(Mat& hsv_image, const int sat_threshold, const int val_threshold, int huebins)
{
    //prune all pixels which don't have significant hue by making a mask, with zeroes indicating unwanted pixels
    vector<Mat> hsv_split(3);
    cv::split(hsv_image, hsv_split);
    Mat hues = hsv_split[0], sats = hsv_split[1], vals = hsv_split[2];
    Mat hue_mask = Mat::ones(hues.size(), hues.type());

    for(int x = 0; x < hues.cols; x ++)
    {
        for(int y = 0; y < hues.rows; y ++)
        {
            if(sats.at<uchar>(y, x) < sat_threshold || vals.at<uchar>(y, x) < val_threshold)
            {
                hue_mask.at<uchar>(y, x) = 0;
            }
        }
    }

    //now we need to make a histogram of the hue channel matrix
    int channels[] = {0}; //we just want the hue channel which is 1st in hsv
    float hrange[] = {0, 180}; //hue ranges from 0 to 180
    const float *ranges[] = { hrange }; //cpp syntax is weird
    int histsize[] = {huebins};
    Mat histogram;
    calcHist(&hues, 1, channels, hue_mask, histogram, 1, histsize, ranges );

    //now get the max values in the histogram and return the corresponding hues
    vector<int> maxhistvals(3, -1); //will contain histogram values
    vector<int> dominanthues(3, -1); //will contain hue values of dominant hues (histogram indexes * binsize)

    MatIterator_<float> it, end;
    for(it = histogram.begin<float>(), end = histogram.end<float>(); it != end; ++it)
    {
        if(*it > maxhistvals[0]) //new max value
        {
            maxhistvals[2] = maxhistvals[1];
            dominanthues[2] = dominanthues[1];
            maxhistvals[1] = maxhistvals[0];
            dominanthues[1] = dominanthues[0];
            maxhistvals[0] = *it;
            dominanthues[0] = (it.pos().y) * (180.0 / huebins);
        }
        else if (*it > maxhistvals[1]) //new 2nd max value
        {
            maxhistvals[2] = maxhistvals[1];
            dominanthues[2] = dominanthues[1];
            maxhistvals[1] = *it;
            dominanthues[1] = (it.pos().y) * (180.0 / huebins);
        }
        else if (*it > maxhistvals[2]) //new 3rd max value
        {
            maxhistvals[2] = *it;
            dominanthues[2] = (it.pos().y) * (180.0 / huebins);
        }
    }

    //edge case where there is very little hue diversity in the image
    if(maxhistvals[1] == 0)
    {
        dominanthues[1] = dominanthues[0];
    }
    if(maxhistvals[2] == 0)
    {
        dominanthues[2] = dominanthues[1];
    }

    return dominanthues;
}

//@TODO parallel?
float getEdginess(Mat grayinput)
{
    MatIterator_<uchar> it, end;
    long int edgecount = 0;

    for( it = grayinput.begin<uchar>(), end = grayinput.end<uchar>(); it != end; ++it)
    {
        if(*it > 150) //it's an edge if it's white
            edgecount++;
    }
    
    return ((float)edgecount) / (grayinput.rows * grayinput.cols);
}


std::string GetMatDepth(const cv::Mat& mat)
{
    const int depth = mat.depth();

    switch (depth)
    {
    case CV_8U:  return "CV_8U";
    case CV_8S:  return "CV_8S";
    case CV_16U: return "CV_16U";
    case CV_16S: return "CV_16S";
    case CV_32S: return "CV_32S";
    case CV_32F: return "CV_32F";
    case CV_64F: return "CV_64F";
    default:
        return "Invalid depth type of matrix!";
    }
}

std::string GetMatType(const cv::Mat& mat)
{
    const int mtype = mat.type();

    switch (mtype)
    {
    case CV_8UC1:  return "CV_8UC1";
    case CV_8UC2:  return "CV_8UC2";
    case CV_8UC3:  return "CV_8UC3";
    case CV_8UC4:  return "CV_8UC4";

    case CV_8SC1:  return "CV_8SC1";
    case CV_8SC2:  return "CV_8SC2";
    case CV_8SC3:  return "CV_8SC3";
    case CV_8SC4:  return "CV_8SC4";

    case CV_16UC1: return "CV_16UC1";
    case CV_16UC2: return "CV_16UC2";
    case CV_16UC3: return "CV_16UC3";
    case CV_16UC4: return "CV_16UC4";

    case CV_16SC1: return "CV_16SC1";
    case CV_16SC2: return "CV_16SC2";
    case CV_16SC3: return "CV_16SC3";
    case CV_16SC4: return "CV_16SC4";

    case CV_32SC1: return "CV_32SC1";
    case CV_32SC2: return "CV_32SC2";
    case CV_32SC3: return "CV_32SC3";
    case CV_32SC4: return "CV_32SC4";

    case CV_32FC1: return "CV_32FC1";
    case CV_32FC2: return "CV_32FC2";
    case CV_32FC3: return "CV_32FC3";
    case CV_32FC4: return "CV_32FC4";

    case CV_64FC1: return "CV_64FC1";
    case CV_64FC2: return "CV_64FC2";
    case CV_64FC3: return "CV_64FC3";
    case CV_64FC4: return "CV_64FC4";

    default:
        return "Invalid type of matrix!";
    }
}