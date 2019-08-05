#include "SamlanMatUtil.h"

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

    // if(downsamplingfactor > 0)
    // {
    //     downsamplingfactor = 2 << downsamplingfactor;
    //     to = from(Range(0, from.rows/downsamplingfactor), Range(0, from.cols/downsamplingfactor));
    //     to = to.clone();

    //     for(int x = 0; x < from.cols; x += downsamplingfactor)
    //     {
    //         for(int y = 0; y < from.rows; y += downsamplingfactor)
    //         {
    //             // cout << x << y << endl;
    //             Vec3b impixel = from.at<Vec3b>(y, x);
    //             Vec3b &downpixel = to.at<Vec3b>(y/downsamplingfactor, x/downsamplingfactor);
    //             for(int k = 0; k < from.channels(); k++)
    //             {
    //                 downpixel.val[k] = impixel.val[k];
    //             }
    //         }
    //     }
    // }
    // else
    // {
    //     to = from;
    // }
}



vector<int> getDominantHues(Mat hue_image, int huebins)
{
    //now we need to make a histogram of the hue channel matrix
    int channels[] = {0}; //we just want the hue channel which is 1st in hsv
    float hrange[] = {0, 180}; //hue ranges from 0 to 180
    const float *ranges[] = { hrange }; //cpp syntax is weird
    int histsize[] = {huebins};
    Mat histogram;
    calcHist(&hue_image, 1, channels, Mat(), histogram, 1, histsize, ranges );

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