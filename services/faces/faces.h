#ifndef FACES_H
#define FACES_H

#include <opencv2/opencv.hpp>
#include <QString>
#include <QImage>
#include <QDebug>

#include "mtcnn.h"
#define MODEL_PATH  "./data/faces/models"
using namespace cv;


class FACES: public QObject
{
    Q_OBJECT
public:
    FACES(QObject *parent=nullptr);

    QImage face_recognition(QImage img);
    MTCNN *mtcnn;
    Mat face_data;
    QImage mat2Qimg(const Mat &mat);
    Mat qimg2Mat(QImage image);
signals:
    void locationInfo(int x1,int y1,int x2, int y2);
private:
    std::vector<Bbox> finalBbox;
    void drawPoint(unsigned char *bits, int width, int depth, int x, int y, const uint8_t *color);
    void drawLine(unsigned char *bits, int width, int depth, int startX, int startY, int endX, int endY, const uint8_t *col);
    void drawRectangle(unsigned char *bits, int width, int depth, int x1, int y1, int x2, int y2, const uint8_t *col);
};
#endif // FACES_H
