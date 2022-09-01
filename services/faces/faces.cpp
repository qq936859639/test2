#include "faces.h"

FACES::FACES(QObject *parent):QObject(parent)
{
    int miniFace = 40;
    mtcnn = new MTCNN(MODEL_PATH);
    mtcnn->SetMinFace(miniFace);
}

void FACES::drawPoint(unsigned char *bits, int width, int depth, int x, int y, const uint8_t *color) {
    for (int i = 0; i < min(depth, 3); ++i) {
        bits[(y * width + x) * depth + i] = color[i];
    }
}

void FACES::drawLine(unsigned char *bits, int width, int depth, int startX, int startY, int endX, int endY,
              const uint8_t *col) {
    if (endX == startX) {
        if (startY > endY) {
            int a = startY;
            startY = endY;
            endY = a;
        }
        for (int y = startY; y <= endY; y++) {
            drawPoint(bits, width, depth, startX, y, col);
        }
    } else {
        float m = 1.0f * (endY - startY) / (endX - startX);
        int y = 0;
        if (startX > endX) {
            int a = startX;
            startX = endX;
            endX = a;
        }
        for (int x = startX; x <= endX; x++) {
            y = (int) (m * (x - startX) + startY);
            drawPoint(bits, width, depth, x, y, col);
        }
    }
}

void FACES::drawRectangle(unsigned char *bits, int width, int depth, int x1, int y1, int x2, int y2, const uint8_t *col) {
    drawLine(bits, width, depth, x1, y1, x2, y1, col);
    drawLine(bits, width, depth, x2, y1, x2, y2, col);
    drawLine(bits, width, depth, x2, y2, x1, y2, col);
    drawLine(bits, width, depth, x1, y2, x1, y1, col);
}

Mat FACES::face_recognition(Mat img) {
    if(img.empty())return img;

    ncnn::Mat ncnn_img = ncnn::Mat::from_pixels(img.data, ncnn::Mat::PIXEL_BGR, img.cols, img.rows);
    mtcnn->detect(ncnn_img, finalBbox);

    if(finalBbox.size() > 0){
        emit locationInfo(finalBbox[0].x1, finalBbox[0].y1,finalBbox[0].x2, finalBbox[0].y2);
        const uint8_t red[3] = {0, 0, 255};
        drawRectangle(img.data, img.cols, 3, finalBbox[0].x1, finalBbox[0].y1,
                      finalBbox[0].x2, finalBbox[0].y2, red);
        finalBbox.clear();
    }
    return img;
}
