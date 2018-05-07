//
//  main.cpp
//  Writing_Recog_System
//
//  Created by zjn on 27/11/2017.
//  Copyright © 2017 zjn. All rights reserved.
//

//opencv libraries
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

//C++ standard libraries
#include <iostream>
#include <vector>

//Customized libraries
#include "topCamera.h"
#include "sideCamera.h"
#include "drawHandler.h"
#include "planeHeightHandler.h"

using namespace cv;
using namespace std;

//function declarations
void skinColorProcess(Mat& cap_frame, Mat& noise_reduced_frame);
void drawingState_callback (int, void* param);
void mouseHandler (int event, int x, int y, int flags, void* param);
bool skRule1(int R, int G, int B);
bool skRule2(float Y, float Cr, float Cb);
bool skRule3(float H, float S, float V);

int main()
{
    // capture video stream from two cameras
    VideoCapture capTop(0);
    VideoCapture capSide(2);
    
    // if not successful, exit program
    if (!capTop.isOpened() || !capSide.isOpened())
    {
        cout << "Cannot open the video cam" << endl;
        return -1;
    }
    
    // create windows for displaying results
    namedWindow("FingerCapture", WINDOW_AUTOSIZE);
    namedWindow("TouchCapture", WINDOW_AUTOSIZE);
    namedWindow("SkinDetection", WINDOW_AUTOSIZE);
    namedWindow("TouchDetection", WINDOW_AUTOSIZE);
    namedWindow("Canvas", WINDOW_AUTOSIZE);
    
    // Declare original image captured by cameras
    Mat cap_frame;
    Mat cap_frame2;
    
    // Declare Mats for accomodating images after detected skin color.
    Mat noise_reduced_frame;
    Mat noise_reduced_frame2;
    
    // Delcare a canvas for drawing purpose
    Mat canvas;
    bool reset_canvas = false;
    vector<Point> draw_path;
    
    // Drawing state parameter
    bool draw_state_changed = false;
    int draw_stroke = 1;
    
    // Height of plane detected
    int plane_height = 177;
    
    // Create a trackbar for adjusting stroke
    createTrackbar("Stroke", "FingerCapture", &draw_stroke, 20, drawingState_callback, &draw_state_changed);
    setMouseCallback("TouchCapture", mouseHandler, &plane_height);
    
    while (1)
    {
        // read a new frame from video
        bool bSuccess = capTop.read(cap_frame);
        bool aSuccess = capSide.read(cap_frame2);
        
        //if not successful, break loop
        if (!bSuccess || !aSuccess)
        {
            cout << "Cannot read a frame from video stream" << endl;
            break;
        }
        
        // Resize two input video stream to improve the performance of the program
        resize(cap_frame, cap_frame, Size(), 0.3, 0.3, CV_INTER_AREA);
        resize(cap_frame2, cap_frame2, Size(), 0.3, 0.3, CV_INTER_AREA);
        
        Mat dst_flip;
        flip(cap_frame, dst_flip, -1);
        dst_flip.copyTo(cap_frame);
        
        // Make sure canvas has been initialized
        if (canvas.empty()) {
            cout << "empty";
            Mat background(cap_frame.rows+30, cap_frame.cols, CV_32FC3, Scalar(255, 255, 255));
            canvas = background;
        }
        
        Point fingertip_pt_top;
        Point fingertip_pt_side;
        
        // Invoke this function to perform skin color detection on both images
        skinColorProcess(cap_frame, noise_reduced_frame);
        skinColorProcess(cap_frame2, noise_reduced_frame2);
        
        int TSuccess = 0;
        int SSuccess = 0;
        bool TouchSuc = 0;
        
        TSuccess = topCamProcess(cap_frame, noise_reduced_frame, fingertip_pt_top);
        SSuccess = sideCamProcess(cap_frame2, noise_reduced_frame2, fingertip_pt_side);
        
        // If draw state is changed, redetect plane height
//        if (draw_state_changed) {
//            plane_height = finddeskHeight(cap_frame2);
//            cout << "current height: " << plane_height << endl;
//        }
        
        // Draw fingertip path on the canvas if draw_path vector is not empty
        if (TSuccess == 1) {
            drawAssist(draw_stroke, draw_path, reset_canvas, draw_state_changed, canvas, fingertip_pt_top);
        }
        if (SSuccess == 1) {
            // Detect touching state
            TouchSuc = touchAssist(cap_frame2, fingertip_pt_side, plane_height);
            if (!TouchSuc) {
                // If fingettip lifts up from plane, clear path data and stop drawing.
                draw_path.clear();
                draw_stroke = 0;
            } else {
                draw_stroke = 1;
            }
        }
        
        imshow("Canvas", canvas);
        imshow("FingerCapture", cap_frame);
        imshow("TouchCapture", cap_frame2);
        imshow("SkinDetection", noise_reduced_frame);
        imshow("TouchDetection", noise_reduced_frame2);
        
        //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
        if (waitKey(30) == 27)
        {
            cout << "esc key is pressed by user" << endl;
            break;
        }
        
    }
    
    capTop.release();
    capSide.release();
    return 0;
}

void drawingState_callback (int, void* param) {
    *(bool *)param = !*(bool *)param;
}

void mouseHandler (int event, int x, int y, int flags, void* param) {
    if (event == CV_EVENT_LBUTTONDOWN) {
        *(int *)param = y;
        cout << "current y: " << y << endl;
    }
}

void skinColorProcess(Mat& cap_frame, Mat& noise_reduced_frame) {
    Mat hsv_frame;
    Mat ycrcb_frame;
    Mat skin_detect_frame;
    
    // Convert original RGB image into HSV color space
    cap_frame.convertTo(hsv_frame, CV_32FC3);
    cvtColor(hsv_frame, hsv_frame, CV_BGR2HSV);
    normalize(hsv_frame, hsv_frame, 0.0, 255.0, NORM_MINMAX, CV_32FC3);
    
    // Convert original RGB image into YCrCb color space
    cvtColor(cap_frame, ycrcb_frame, CV_BGR2YCrCb);
    
    skin_detect_frame = Mat::zeros(cap_frame.rows, cap_frame.cols, CV_8UC1);
    
    // Exert constraints on the original image to identify the skin color region
    for (int i = 0; i < cap_frame.rows; i++) {
        for (int j = 0; j < cap_frame.cols; j++) {
            Vec3b intensity = cap_frame.at<Vec3b>(i, j);
            int B = intensity[0];
            int G = intensity[1];
            int R = intensity[2];
            
            Vec3f HSV_val = hsv_frame.at<Vec3f>(i, j);
            float H = HSV_val[0];
            float S = HSV_val[1];
            float V = HSV_val[2];
            
            Vec3b YCRCB_val = ycrcb_frame.at<Vec3b>(i, j);
            int Y = YCRCB_val[0];
            int Cr = YCRCB_val[1];
            int Cb = YCRCB_val[2];
            
            bool r1 = skRule1(R, G, B);
            bool r2 = skRule2(Y, Cr, Cb);
            bool r3 = skRule3(H, S, V);
            
            if (r1 && r2 && r3) {
                skin_detect_frame.at<uchar>(i, j) = 255;
            }
        }
    }
    skin_detect_frame.copyTo(noise_reduced_frame);
    
    // Apply median filter on processed image to eliminate noise
    medianBlur(noise_reduced_frame, noise_reduced_frame, 5);
    
    // Apply dilation method on processed image to fill holes
    int element_size = 5;
    Mat element = getStructuringElement(MORPH_ELLIPSE, Size(2 * element_size + 1, 2 * element_size + 1), Point(element_size, element_size));
    dilate(noise_reduced_frame, noise_reduced_frame, element);
}

// First constraint on RGB color space
bool skRule1(int R, int G, int B) {
    bool e1 = (R>95) && (G>40) && (B>20) && ((max(R,max(G,B)) - min(R, min(G,B)))>15) && (abs(R-G)>15) && (R>G) && (R>B);
    bool e2 = (R>220) && (G>210) && (B>170) && (abs(R-G)<=15) && (R>B) && (G>B);
    
    return (e1||e2);
}

// Second constraint on YCbCr color space
bool skRule2(float Y, float Cr, float Cb) {
    bool e3 = Cr <= 1.5862*Cb+20;
    bool e4 = Cr >= 0.3448*Cb+76.2069;
    bool e5 = Cr >= -4.5652*Cb+234.5652;
    bool e6 = Cr <= -1.15*Cb+301.75;
    bool e7 = Cr <= -2.2857*Cb+432.85;
    
    return e3 && e4 && e5 && e6 && e7;
}

// Third constraint on HSV color space
bool skRule3(float H, float S, float V) {
    return (H<25) || (H > 230);
}
