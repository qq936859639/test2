#ifndef PLR_H
#define PLR_H

#include <vector>
#include <iostream>
#include "opencv2/core.hpp"
#include <opencv2/opencv.hpp>
#include "plr/include/plate_detectors.h"
#include "plr/include/plate_recognizers.h"
#include "cvUniText.hpp"
#include "timing.h"

using namespace cv;
using namespace std;
using namespace pr;

class PLR {
public:
    PLR();
    Mat  test_mtcnn_plate(Mat img);
    string LPR_Data;
private:
    PlateDetector detector;
    LPRRecognizer lpr ;
    Mat draw_plate_results(std::vector<pr::PlateInfo> &objects, cv::Mat &image);
};

#endif // PLR_H
