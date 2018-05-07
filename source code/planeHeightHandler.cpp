#include "planeHeightHandler.h"
#include "drawHandler.h"

int finddeskHeight(Mat& src_org){
    Mat src;
    
    src_org.copyTo(src);
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	Mat roi = src(Rect(0.3*src.cols, 0.7*src.rows, 0.1*src.cols, 0.1*src.rows));  // take ROI from image
	Mat hsv, hsvt, roihist;
	// Convert ROI image to HSV format
	cvtColor(roi, hsv, COLOR_BGR2HSV);
	// Convert source image to HSV format
	cvtColor(src, hsvt, COLOR_BGR2HSV);

	int histSize[] = { 32, 32, 32 };
	float hRanges[] = { 0, 180 };
	float sRanges[] = { 0, 256 };
	float vRanges[] = { 0, 256 };
	const float* ranges[] = { hRanges, sRanges, vRanges };
	int channels[] = { 0, 1, 2 };

	// Find the histograms using calcHist
	calcHist(&hsv, 1, channels, Mat(), roihist, 3, histSize, ranges);
	Mat dst, thr;
	calcBackProject(&hsvt, 1, channels, roihist, dst, ranges, 1);
	Mat disc = getStructuringElement(MORPH_ELLIPSE, Point(8, 8));
	filter2D(dst, dst, -1, disc);

	int largest_area = 0;
	int largest_contour_index = 0;
	Rect bounding_rect;

	
	threshold(dst, thr, 180, 255, THRESH_BINARY);
    
	findContours(thr, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

	for (int i = 0; i< contours.size(); i++) // iterate through each contour.
	{
		double a = contourArea(contours[i], false);  //  Find the area of contour
		if (a>largest_area){
			largest_area = a;
			largest_contour_index = i;                //Store the index of largest contour
			bounding_rect = boundingRect(contours[i]); // Find the bounding rectangle for biggest contour
		}
	}

	drawContours(src, contours, -1, Scalar(255, 0, 255), 3);
	rectangle(src, bounding_rect, Scalar(255, 255, 255), 2, 8, 0);
    
    int height = bounding_rect.height;
    
    char text[50];
    sprintf(text, "Current height: %d", height);
    drawText(src, text);
    
    namedWindow("HeightDetection");
    imshow("HeightDetection", src);
    return height;
}
