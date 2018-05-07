//
//  topCamera.cpp
//  Writing_Recog_System
//
//  Created by zjn on 05/12/2017.
//  Copyright © 2017 zjn. All rights reserved.
//

#include "topCamera.h"

int topCamProcess(Mat& cap_frame, Mat& noise_reduced_frame, Point& fingertip_pt) {
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
        
        // group neighboring points in set into vector
        for (int i = 0; i < hull[0].size()-1; i++) {
            Point p1 = hull[0][i];
            Point p2 = hull[0][i+1];
            double ptDist = sqrt(pow(p1.x-p2.x, 2) + pow(p1.y-p2.y, 2));
            
            areaHullPts.push_back(p1);
//            circle(cap_frame, p1, 5, CV_RGB(255, 0, 0), 3);
            
            // If the distance to the next point is too faraway, store current point group in a vector and initialize the container
            if (ptDist > 20) {
                reducedHullPts.push_back(areaHullPts);
                areaHullPts.clear();
//                circle(cap_frame, p1, 5, CV_RGB(255, 255, 0), 3);
//                continue;
            }
//            circle(cap_frame, p1, 5, CV_RGB(255, 0, 0), 3);
        }
        
        // Clear container for further use
        areaHullPts.clear();
        
        // Only take into account the point with median index in a set of neighboring points
        for (int i = 0; i < reducedHullPts.size(); i++) {
            int mid = float(reducedHullPts[i].size() / 2) + 1;
//            mid = 0;
            areaHullPts.push_back(reducedHullPts[i][mid]);
//            circle(cap_frame, reducedHullPts[i][mid], 5, CV_RGB(255, 0, 0), 3);
        }
        
        // Draw bounding box of the coutour
        Rect handBoundingBox = boundingRect(hull[0]);
        rectangle(cap_frame, handBoundingBox, CV_RGB(0, 255, 255));
        
        // Calculate and draw the center of bounding box
        Point contour_center = Point(handBoundingBox.x + handBoundingBox.width/2, handBoundingBox.y + handBoundingBox.height/2);
        circle(cap_frame, contour_center, 5, CV_RGB(255, 255, 255), 3);
        
        // Filter out those points that do not conform to angle constraint
        for (int i = 0; i < areaHullPts.size(); i++) {
            Point pt = areaHullPts[i];
            double angle_with_center = atan2(contour_center.y - pt.y, pt.x - contour_center.x) * 180 / CV_PI;
            if (angle_with_center > 40 && angle_with_center < 140) {
                validHullPts.push_back(pt);
//                circle(cap_frame, pt, 5, CV_RGB(255, 0, 0), 3);
                //                    cout << "size: " << validHullPts.size() << endl;
            }
        }
        
        if (!validHullPts.empty()) {
            int optimal_valid_id = 0;
            
            // Figure out the leftover points that in accordance with the second constraint
            for (int i = 0; i < validHullPts.size(); i++) {
                int y_diff = contour_center.y - validHullPts[i].y;
                if (y_diff <= handBoundingBox.height/2 + 10 && y_diff > handBoundingBox.height/2 - 10) {
                    optimal_valid_id = i;
                    break;
                }
            }
            
//            cout << "current optID: " << optimal_valid_id << endl;
            
            // Obtain the final result
            fingertip_pt = validHullPts[optimal_valid_id];
            circle(cap_frame, fingertip_pt, 5, CV_RGB(255, 0, 0), 3);
            return 1;
        }
        return 0;
    }
    return 0;
}
