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
#include "topCamera.cpp"

using namespace cv;
using namespace std;

//function declarations
bool skRule1(int R, int G, int B);
bool skRule2(float Y, float Cr, float Cb);
bool skRule3(float H, float S, float V);
void object_reduction(Mat& src_img, int cur_row, int cur_col, int& pxl_count, vector<Point>& obj_pt);
void mouseHandler (int event, int x, int y, int flags, void* param);
void drawText(Mat& img_display, string text_content);
void drawingState_callback (int, void* param);

int main()
{
    VideoCapture cap(0);
    
    // if not successful, exit program
    if (!cap.isOpened())
    {
        cout << "Cannot open the video cam" << endl;
        return -1;
    }
    
    //create a window called "MyVideoFrame0"
    namedWindow("FingerCapture", WINDOW_AUTOSIZE);
    namedWindow("SkinDetection", WINDOW_AUTOSIZE);
    namedWindow("Canvas", WINDOW_AUTOSIZE);
    
    Mat cap_frame;
    Mat hsv_frame;
    Mat ycrcb_frame;
    Mat skin_detect_frame;
    Mat noise_reduced_frame;
    
    Mat canvas;
    bool reset_canvas = true;
    vector<Point> draw_path;
    
    bool draw_state_changed = false;
//    bool draw_state = false;
//    setMouseCallback("Canvas", mouseHandler, &draw_state_changed);
    int draw_stroke = 0;
    createTrackbar("Stroke", "FingerCapture", &draw_stroke, 20, drawingState_callback, &draw_state_changed);
    
    while (1)
    {
        // read a new frame from video
        bool bSuccess = cap.read(cap_frame);
        
        //if not successful, break loop
        if (!bSuccess)
        {
            cout << "Cannot read a frame from video stream" << endl;
            break;
        }
        
        resize(cap_frame, cap_frame, Size(), 0.3, 0.3, CV_INTER_AREA);
        
        cap_frame.convertTo(hsv_frame, CV_32FC3);
        cvtColor(hsv_frame, hsv_frame, CV_BGR2HSV);
        normalize(hsv_frame, hsv_frame, 0.0, 255.0, NORM_MINMAX, CV_32FC3);
        
        cvtColor(cap_frame, ycrcb_frame, CV_BGR2YCrCb);
        
        skin_detect_frame = Mat::zeros(cap_frame.rows, cap_frame.cols, CV_8UC1);
        
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
//        noise_reduced_frame = Mat::zeros(skin_detect_frame.rows, skin_detect_frame.cols, CV_8UC1);
//
//        for (int i = 0; i < skin_detect_frame.rows; i++) {
//            for (int j = 0; j < skin_detect_frame.cols; j++) {
//                if (skin_detect_frame.at<uchar>(i, j) == 255) {
//                    vector<Point> obj_pt;
//                    int pxl_count = 0;
//                    // Start from an initial point and detect the whole object recursively
//                    object_reduction(skin_detect_frame, i, j, pxl_count, obj_pt);
//
//                    // Dominant objects should occupy certain amount of areas
//                    if (pxl_count > 50) {
//                        for (int vi = 0; vi < obj_pt.size(); vi++) {
//                            noise_reduced_frame.at<uchar>(obj_pt[vi]) = 255;
//                        }
//                    }
//                }
//            }
//        }
        
        // Apply median filter on processed image to eliminate noise
        medianBlur(noise_reduced_frame, noise_reduced_frame, 5);
        
        // Apply dilation method on processed image to fill holes
        int element_size = 5;
        Mat element = getStructuringElement(MORPH_ELLIPSE, Size(2 * element_size + 1, 2 * element_size + 1), Point(element_size, element_size));
        dilate(noise_reduced_frame, noise_reduced_frame, element);
        
        vector<vector<Point>> contours;
        vector<Vec4i> hierarchy;
        findContours(noise_reduced_frame, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
        
        int largestContour = 0;
        for (int i = 1; i < contours.size(); i++) {
            if (contourArea(contours[i]) > contourArea(contours[largestContour]))
                largestContour = i;
        }
        drawContours(cap_frame, contours, largestContour, Scalar(0, 0, 255));
        
        if (!contours.empty())
        {
            vector<vector<Point> > hull(1);
            convexHull(Mat(contours[largestContour]), hull[0], false);
            drawContours(cap_frame, hull, 0, cv::Scalar(0, 255, 0), 3);
            
            vector<vector<Point>> reducedHullPts;
            vector<Point> areaHullPts;
            vector<Point> validHullPts;
            int areaHullCount = 0;
            
            for (int i = 0; i < hull[0].size()-1; i++) {
                Point p1 = hull[0][i];
                Point p2 = hull[0][i+1];
                double ptDist = sqrt(pow(p1.x-p2.x, 2) + pow(p1.y-p2.y, 2));
                
                areaHullPts.push_back(p1);
                
                if (ptDist > 20) {
                    reducedHullPts.push_back(areaHullPts);
                    areaHullPts.clear();
                }
            }
            
            areaHullPts.clear();
            
            for (int i = 0; i < reducedHullPts.size(); i++) {
                int mid = float(reducedHullPts[i].size() / 2) + 1;
                areaHullPts.push_back(reducedHullPts[i][mid]);
            }
            
//            for (int i = 0; i < areaHullPts.size(); i++) {
//                circle(cap_frame, areaHullPts[i], 5, CV_RGB(255, 0, 0), 3);
//            }
            
            Rect handBoundingBox = boundingRect(hull[0]);
            rectangle(cap_frame, handBoundingBox, CV_RGB(0, 255, 255));
            Point contour_center = Point(handBoundingBox.x + handBoundingBox.width/2, handBoundingBox.y + handBoundingBox.height/2);
            circle(cap_frame, contour_center, 5, CV_RGB(255, 255, 255), 3);
            
            for (int i = 0; i < areaHullPts.size(); i++) {
                Point pt = areaHullPts[i];
                double angle_with_center = atan2(contour_center.y - pt.y, contour_center.x - pt.x) * 180 / CV_PI;
                if (angle_with_center > 50 && angle_with_center < 130) {
                    validHullPts.push_back(pt);
//                    cout << "size: " << validHullPts.size() << endl;
                }
            }
            
//            for (int i = 0; i < validHullPts.size(); i++)
//                circle(cap_frame, validHullPts[i], 5, CV_RGB(255, 0, 0), 3);
            
            if (!validHullPts.empty()) {
                int optimal_valid_id = 0;
                
                for (int i = 0; i < validHullPts.size(); i++) {
                    int y_diff = contour_center.y - validHullPts[i].y;
                    if (y_diff <= handBoundingBox.height/2 + 10) {
                        optimal_valid_id = i;
                        break;
                    }
                }
//                cout << "current optID: " << optimal_valid_id << endl;
                Point fingertip_pt = validHullPts[optimal_valid_id];
                circle(cap_frame, fingertip_pt, 5, CV_RGB(255, 0, 0), 3);
                if (draw_stroke == 0) {
                }
                else if (draw_path.size() == 0) {
                    draw_path.push_back(fingertip_pt);
                }
                else {
                    int end_id = int(draw_path.size()-1);
                    double ptDist = sqrt(pow(fingertip_pt.x-draw_path[end_id].x, 2) + pow(fingertip_pt.y-draw_path[end_id].y, 2));
                    if (ptDist < 300) {
                        draw_path.push_back(fingertip_pt);
                        line(canvas, draw_path[end_id], draw_path[end_id+1], CV_RGB(0, 0, 0), 2);
                    }
                }
            }
        }
        
        if (reset_canvas) {
            Mat background(cap_frame.rows+30, cap_frame.cols, CV_32FC3, Scalar(255, 255, 255));
            canvas = background;
            draw_path.clear();
            reset_canvas = false;
        }
        
        if (draw_state_changed) {
            draw_state_changed = false;
            reset_canvas = true;
            
            Point text_topleft(canvas.cols - 151, canvas.rows - 31);
            Point text_bottomright(canvas.cols - 1, canvas.rows - 1);
            rectangle(canvas, text_topleft, text_bottomright, CV_RGB(255, 255, 255), -1);
        }
        
        if (draw_stroke > 0) {
            drawText(canvas, "Drawing enabled");
        } else {
            drawText(canvas, "Drawing disabled");
        }
        
        imshow("Canvas", canvas);
        imshow("FingerCapture", cap_frame);
        imshow("SkinDetection", noise_reduced_frame);
        
        //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
        if (waitKey(30) == 27)
        {
            cout << "esc key is pressed by user" << endl;
            break;
        }
        
    }
    
    cap.release();
    return 0;
}

//void mouseHandler (int event, int x, int y, int flags, void* param) {
//    switch (event) {
//        case CV_EVENT_LBUTTONDOWN:
//            cout << "x: " << x << "  y:" << y << endl;
//            *(bool *)param = !*(bool *)param;
//            break;
//
//        default:
//            break;
//    }
//}

void drawingState_callback (int, void* param) {
    *(bool *)param = !*(bool *)param;
}

void drawText(Mat& img_display, string text_content) {
    int font_face = FONT_HERSHEY_PLAIN;
    double font_scale = 1;
    int thickness = 2;
    int baseline;
    
    Size text_size = getTextSize(text_content, font_face, font_scale, thickness, &baseline);
    
    Point origin_pos;
    origin_pos.x = img_display.cols - 10 - text_size.width;
    origin_pos.y = img_display.rows - text_size.height;
    putText(img_display, text_content, origin_pos, font_face, font_scale, Scalar(0, 0, 255), thickness, 8, 0);
}

void object_reduction(Mat& src_img, int cur_row, int cur_col, int& pxl_count, vector<Point>& obj_pt) {
    if (cur_row <= 0 || cur_col <= 0 || cur_row >= src_img.rows-1 || cur_col >= src_img.cols-1) {
        return;
    }
    if (src_img.at<uchar>(cur_row, cur_col) < 255) {
        return;
    }
    
    pxl_count++;
    obj_pt.push_back(Point_<int>(cur_col, cur_row));
        
    src_img.at<uchar>(cur_row, cur_col) = 0;
    object_reduction(src_img, cur_row, cur_col+1, pxl_count, obj_pt);
    object_reduction(src_img, cur_row, cur_col-1, pxl_count, obj_pt);
    object_reduction(src_img, cur_row+1, cur_col, pxl_count, obj_pt);
    object_reduction(src_img, cur_row-1, cur_col, pxl_count, obj_pt);
}

bool skRule1(int R, int G, int B) {
    bool e1 = (R>95) && (G>40) && (B>20) && ((max(R,max(G,B)) - min(R, min(G,B)))>15) && (abs(R-G)>15) && (R>G) && (R>B);
    bool e2 = (R>220) && (G>210) && (B>170) && (abs(R-G)<=15) && (R>B) && (G>B);
    
    return (e1||e2);
}

bool skRule2(float Y, float Cr, float Cb) {
    bool e3 = Cr <= 1.5862*Cb+20;
    bool e4 = Cr >= 0.3448*Cb+76.2069;
    bool e5 = Cr >= -4.5652*Cb+234.5652;
    bool e6 = Cr <= -1.15*Cb+301.75;
    bool e7 = Cr <= -2.2857*Cb+432.85;
    
    return e3 && e4 && e5 && e6 && e7;
}

bool skRule3(float H, float S, float V) {
    return (H<25) || (H > 230);
}
