//
//  sideCamera.h
//  Writing_Recog_System
//
//  Created by zjn on 05/12/2017.
//  Copyright © 2017 zjn. All rights reserved.
//

#ifndef sideCamera_h
#define sideCamera_h

//opencv libraries
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

//C++ standard libraries
#include <iostream>
#include <vector>

using namespace cv;
using namespace std;

int sideCamProcess(Mat& cap_frame, Mat& noise_reduced_frame, Point& fingertip_pt);

#endif /* sideCamera_h */
