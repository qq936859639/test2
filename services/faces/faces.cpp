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

QImage FACES::face_recognition(QImage img) {
    Mat srcimg = qimg2Mat(img);
    if(srcimg.empty())
        return img;

    ncnn::Mat ncnn_img = ncnn::Mat::from_pixels(srcimg.data, ncnn::Mat::PIXEL_BGR, img.width(),img.height());
    mtcnn->detect(ncnn_img, finalBbox);

    if(finalBbox.size() > 0){
        emit locationInfo(finalBbox[0].x1, finalBbox[0].y1,finalBbox[0].x2, finalBbox[0].y2);
        const uint8_t red[3] = {0, 0, 255};
        drawRectangle(srcimg.data, img.width(), 3, finalBbox[0].x1, finalBbox[0].y1,
                      finalBbox[0].x2, finalBbox[0].y2, red);
        finalBbox.clear();
    }
    QImage result = mat2Qimg(srcimg);
    return result;
}
cv::Mat FACES::qimg2Mat(QImage image)
{
    Mat mat;
    int format = image.format();
    switch (format)
    {
    case QImage::Format_RGB32:
        mat = cv::Mat(image.height(), image.width(), CV_8UC4, (void*)image.constBits(), image.bytesPerLine());
        cvtColor(mat,mat,COLOR_BGRA2BGR);
        break;
    case QImage::Format_RGB888:
        mat = cv::Mat(image.height(), image.width(), CV_8UC3, (void*)image.constBits(), image.bytesPerLine());
        cvtColor(mat,mat,COLOR_RGB2BGR);
        break;
    case QImage::Format_Indexed8:
        mat = Mat(image.height(), image.width(), CV_8UC1, (void*)image.constBits(), image.bytesPerLine());
        break;
    }
    return mat;
}
QImage FACES::mat2Qimg(const Mat &mat)
{
    QImage image;
    switch (mat.type())
    {
    // 8-bit, 4 channel
    case CV_8UC4:
    {
        image = QImage(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_ARGB32);
        return image;
    }
        // 8-bit, 3 channel
    case CV_8UC3:
    {
        image = QImage(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
        return image.rgbSwapped();
    }
        // 8-bit, 1 channel
    case CV_8UC1:
    {
#if QT_VERSION < QT_VERSION_CHECK(5, 5, 0)
        image = QImage(mat.data, mat.cols, mat.rows, int(mat.step), QImage::Format_Grayscale8);
#else
        QVector<QRgb> sColorTable;
        if (sColorTable.isEmpty())
        {
            sColorTable.resize( 256 );
            for ( int i =0; i <256; ++i )
            {
                sColorTable[i] = qRgb( i, i, i );
            }
        }
        image = QImage(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_Indexed8 );
        image.setColorTable(sColorTable);
#endif
        return image;
    }
    default:
        qDebug() << "ERROR: Mat could not be converted to QImage.";
        break;
    }
    return image;
}
