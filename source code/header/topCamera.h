//
//  topCamera.h
//  Writing_Recog_System
//
//  Created by zjn on 05/12/2017.
//  Copyright © 2017 zjn. All rights reserved.
//

#ifndef topCamera_h
#define topCamera_h

//opencv libraries
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

//C++ standard libraries
#include <iostream>
#include <vector>

using namespace cv;
using namespace std;

int topCamProcess(Mat& cap_frame, Mat& noise_reduced_frame, Point& fingertip_pt);


#endif /* topCamera_h */
