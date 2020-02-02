#include "FancyDisplayCanvas.h"

FancyDisplayCanvas::FancyDisplayCanvas(Mat &camimage, string windowname) :
    window_name(windowname),
    canvas(camimage.clone())
{
    initializeColorBoxes();
}

void FancyDisplayCanvas::draw()
{
    Mat temp;
    cv::resize(small_canvas, temp, canvas.size(), 0, 0, INTER_NEAREST);
    this->add(temp);
    small_canvas = Mat::zeros(small_canvas.size(), CV_8UC3);
    Size stemp = canvas.size();
    resize(canvas, canvas, Size(1000, 1000), 0, 0, INTER_LINEAR);
    imshow(window_name, canvas);
    resize(canvas, canvas, stemp, 0, 0, INTER_LINEAR);
}

void FancyDisplayCanvas::clearToNewImage(Mat& camimage)
{
    canvas = camimage.clone();
}

void FancyDisplayCanvas::add(Mat& image, float alpha)
{
    if(alpha > 0.999) cv::add(canvas, image, canvas);
    else cv::addWeighted(canvas, 1, image, alpha, 0, canvas);
}

void FancyDisplayCanvas::addSmall(Mat& image, float alpha)
{
    if(small_canvas.empty())
    {
        small_canvas = Mat::zeros(image.size(), CV_8UC3);
    }

    if(alpha > 0.999) cv::add(image, small_canvas, small_canvas);
    else cv::addWeighted(small_canvas, 1, image, alpha, 0, small_canvas);
    
}

void FancyDisplayCanvas::makeBlackImageRectangle()
{
    fillPoly(canvas, nocolor_rect, Scalar(0, 0, 0) );
}
void FancyDisplayCanvas::makeWhiteImageRectangle()
{
    fillPoly(canvas, nocolor_rect, Scalar(255, 255, 255) );
}
void FancyDisplayCanvas::makeGrayImageRectangle()
{
    fillPoly(canvas, nocolor_rect, Scalar(80, 80, 80) );
}
void FancyDisplayCanvas::makeColoredImageRectangle(vector<int> &dominant_hues)
{
    //we need to convert the dominant hues to bgr to display them on the gui
    vector<Vec3b> maxhueshsv(3);
    vector<Vec3b> maxhuesbgr(3);
    maxhueshsv[0] = Vec3b(dominant_hues[0], 255, 255);
    maxhueshsv[1] = Vec3b(dominant_hues[1], 255, 255);
    maxhueshsv[2] = Vec3b(dominant_hues[2], 255, 255);
    cvtColor(maxhueshsv, maxhuesbgr, COLOR_HSV2BGR);

    fillPoly(canvas, hue_rect1, maxhuesbgr[0] );
    fillPoly(canvas, hue_rect2, maxhuesbgr[1] );
    fillPoly(canvas, hue_rect3, maxhuesbgr[2] );
}

void FancyDisplayCanvas::addOpticalFlowVisual(OpticalFlowMat& optical_flow)
{
    Mat flow_parts[2], flowmagnitude, angle;
    split(optical_flow, flow_parts);
    cartToPolar(flow_parts[0], flow_parts[1], flowmagnitude, angle, true);
    flowmagnitude /= optical_flow.getFastMovementSize() / 1.5;
    //@@@TODO maybe try to optimize this so it doesn't have to do extra conversions from range 0-1 to range 0-255
    angle *= ((1.f / 360.f) * (180.f / 255.f));
    // build hsv image
    Mat _hsv[3], hsv, hsv8;
    _hsv[0] = angle;
    _hsv[1] = Mat::ones(angle.size(), CV_32F);
    _hsv[2] = flowmagnitude;
    merge(_hsv, 3, hsv);
    hsv.convertTo(hsv8, CV_8U, 255.0);
    //convert to bgr and add to output image
    Mat flow_bgr;
    cvtColor(hsv8, flow_bgr, COLOR_HSV2BGR);
    addSmall(flow_bgr);
}

void FancyDisplayCanvas::initializeColorBoxes()
{
    nocolor_rect = vector<vector<Point>>(1, vector<Point>(4));
    hue_rect1 = vector<vector<Point>>(1, vector<Point>(4));
    hue_rect2 = vector<vector<Point>>(1, vector<Point>(4));
    hue_rect3 = vector<vector<Point>>(1, vector<Point>(4));

    int xleft = (1 * canvas.cols) / 8;
    int ytop = 3 * canvas.rows / 4;
    int xwidth = 3 *canvas.cols / 4;
    int yheight = canvas.rows / 4;
    
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