//
//  drawHandler.h
//  Writing_Recog_System
//
//  Created by zjn on 05/12/2017.
//  Copyright © 2017 zjn. All rights reserved.
//

#ifndef drawHandler_h
#define drawHandler_h

//opencv libraries
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

//C++ standard libraries
#include <iostream>
#include <vector>

using namespace cv;
using namespace std;

void drawText(Mat& img_display, string text_content);
void drawAssist(int draw_stroke, vector<Point>& draw_path, bool& reset_canvas, bool& draw_state_changed, Mat& canvas, Point& fingertip_pt);
bool touchAssist (Mat& cap_frame, Point& fingertip_pt, int plane_height);

#endif /* drawHandler_h */

