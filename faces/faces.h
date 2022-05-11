#ifndef FACES_H
#define FACES_H

#include <opencv2/opencv.hpp>
#include "mtcnn.h"
using namespace cv;

class FACES {
public:
    FACES();
//    Mat  test_mtcnn_plate(Mat img);
//    string LPR_Data;
    Mat face_recognition(Mat img);
    MTCNN *mtcnn;
    Mat face_data;
private:

//    PlateDetector detector;
//    LPRRecognizer lpr ;

//    Mat draw_plate_results(std::vector<pr::PlateInfo> &objects, cv::Mat &image);

    /*
    void facialPoseCorrection(unsigned char *inputImage, int Width, int Height, int Channels, int left_eye_x,
                              int left_eye_y,
                              int right_eye_x, int right_eye_y);
    void RotateBilinear(unsigned char *srcData, int srcWidth, int srcHeight, int Channels, int srcStride,
                        unsigned char *dstData, int dstWidth, int dstHeight, int dstStride, float degree,
                        int fillColorR = 255, int fillColorG = 255, int fillColorB = 255);
    void
    RemoveRedEyes(unsigned char *input, unsigned char *output, int width, int height, int depth, int CenterX, int CenterY,
                  int Radius);*/
};
#endif // FACES_H
