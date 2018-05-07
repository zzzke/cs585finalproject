//
//  sideCamera.cpp
//  Writing_Recog_System
//
//  Created by zjn on 05/12/2017.
//  Copyright © 2017 zjn. All rights reserved.
//

#include "sideCamera.h"

int sideCamProcess(Mat& cap_frame, Mat& noise_reduced_frame, Point& fingertip_pt) {
    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    
    // Find contours from the input frame
    findContours(noise_reduced_frame, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
    
    // Calculate the largest contour from the contour set
    int largestContour = 0;
    for (int i = 1; i < contours.size(); i++) {
        if (contourArea(contours[i]) > contourArea(contours[largestContour]))
            largestContour = i;
    }
    drawContours(cap_frame, contours, largestContour, Scalar(0, 0, 255));
    
    if (!contours.empty()) {
        // Calculate the convex hull of the largest contour
        vector<vector<Point> > hull(1);
        convexHull(Mat(contours[largestContour]), hull[0], false);
        drawContours(cap_frame, hull, 0, cv::Scalar(0, 255, 0), 3);
        
        vector<vector<Point>> reducedHullPts;
        vector<Point> areaHullPts;
        vector<Point> validHullPts;
        
        // Draw bounding box of the coutour
        Rect handBoundingBox = boundingRect(hull[0]);
        rectangle(cap_frame, handBoundingBox, CV_RGB(0, 255, 255));
        
        // Calculate and draw the center of bounding box
        Point contour_center = Point(handBoundingBox.x + handBoundingBox.width/2, handBoundingBox.y + handBoundingBox.height/2);
        circle(cap_frame, contour_center, 5, CV_RGB(255, 255, 255), 3);
        
        // Filter out those points above the center of bounding box and beneath the lower bound of the boudning box
        for (int i = 1; i < hull[0].size(); i++) {
            Point p1 = hull[0][i];
            int y_diff = p1.y - contour_center.y;
            if (y_diff > 0 && y_diff <= handBoundingBox.height/2 + 10) {
                areaHullPts.push_back(hull[0][i]);
            }
        }
        
        // Filter out those points that do not conform to angle constraint
        for (int i = 0; i < areaHullPts.size(); i++) {
            Point pt = areaHullPts[i];
            double angle_with_center = atan2(contour_center.y - pt.y, contour_center.x - pt.x) * 180 / CV_PI;
            if (angle_with_center < -20 && angle_with_center > -70) {
//            if (angle_with_center < -110 && angle_with_center > -160) {
                validHullPts.push_back(pt);
                //                    cout << "size: " << validHullPts.size() << endl;
            }
        }
        
//        for (int i = 0; i < validHullPts.size(); i++)
//            circle(cap_frame, validHullPts[i], 5, CV_RGB(255, 0, 0), 3);
        
        if (!validHullPts.empty()) {
            int optimal_valid_id = 0;
            int y_diff_highest = 0;
            
            // Figure out the leftover points that in accordance with the second constraint
            for (int i = 0; i < validHullPts.size(); i++) {
                int y_diff = validHullPts[i].y - contour_center.y;
                if (y_diff <= handBoundingBox.height/2 + 10 && y_diff > y_diff_highest) {
                    optimal_valid_id = i;
                    y_diff_highest = y_diff;
                }
            }
            //                cout << "current optID: " << optimal_valid_id << endl;
            fingertip_pt = validHullPts[optimal_valid_id];
            circle(cap_frame, fingertip_pt, 5, CV_RGB(255, 0, 0), 3);
            return 1;
        }
        return 0;
    }
    return 0;
}
