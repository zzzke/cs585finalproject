//
//  drawHandler.cpp
//  Writing_Recog_System
//
//  Created by zjn on 05/12/2017.
//  Copyright © 2017 zjn. All rights reserved.
//

#include "drawHandler.h"

void drawAssist(int draw_stroke, vector<Point>& draw_path, bool& reset_canvas, bool& draw_state_changed, Mat& canvas, Point& fingertip_pt) {
    if (draw_stroke == 0) {
        // If draw stroke is 0, stop drawing and do nothing
    }
    else if (draw_path.size() == 0) {
        // If touching state was detected and draw path has no data, unconditionally fill it with current fingertip position
        draw_path.push_back(fingertip_pt);
    }
    else {
        int end_id = int(draw_path.size()-1);
        double ptDist = sqrt(pow(fingertip_pt.x-draw_path[end_id].x, 2) + pow(fingertip_pt.y-draw_path[end_id].y, 2));
        
        // If current tracing position is far away from the last position, treat this point as an exception
        // and simply ignore it
        if (ptDist < 300) {
            draw_path.push_back(fingertip_pt);
            // draw line on the canvas
            line(canvas, draw_path[end_id], draw_path[end_id+1], CV_RGB(0, 0, 0), 2);
        }
    }
    
    // If user refresh the canvas, create a new background and replace canvas with this new background
    if (reset_canvas) {
        Mat background(canvas.rows, canvas.cols, CV_32FC3, Scalar(255, 255, 255));
        canvas = background;
        draw_path.clear();
        reset_canvas = false;
        
        // Show current drawing state on canvas
        if (draw_stroke > 0) {
            drawText(canvas, "Drawing enabled");
        } else {
            drawText(canvas, "Drawing disabled");
        }
    }
    
    // If user change the drawing state
    if (draw_state_changed) {
        draw_state_changed = false;
        reset_canvas = true;
        
        // Erase old drawing state from canvas
        Point text_topleft(canvas.cols - 151, canvas.rows - 31);
        Point text_bottomright(canvas.cols - 1, canvas.rows - 1);
        rectangle(canvas, text_topleft, text_bottomright, CV_RGB(255, 255, 255), -1);
    }
}

bool touchAssist (Mat& cap_frame, Point& fingertip_pt, int plane_height) {
    // Calculate if fingertip is close enough to the plane
    if (plane_height > 0 && fingertip_pt.y > plane_height - 10) {
        drawText(cap_frame, "Finger Touch!");
        return true;
    }
    return false;
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
