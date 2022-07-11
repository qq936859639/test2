#include "lpr.h"
PLR::PLR(){
    fix_mtcnn_detector("lpr/models/float", pr::mtcnn_float_detector);
    detector = pr::IPlateDetector::create_plate_detector(pr::mtcnn_float_detector);
    fix_lpr_recognizer("lpr/models/float", pr::float_lpr_recognizer);
    lpr = pr::float_lpr_recognizer.create_recognizer();
}

cv::Mat PLR::draw_plate_results(std::vector<PlateInfo> &objects, cv::Mat &image) {
    uni_text::UniText uniText("font/wqy-microhei.ttc",40);
    for (auto pi : objects) {
        cv::Rect area(pi.bbox.xmin, pi.bbox.ymin, pi.bbox.xmax - pi.bbox.xmin, pi.bbox.ymax - pi.bbox.ymin);
        cv::rectangle(image, area, cv::Scalar(255, 0, 0));
        int baseLine = 0;
        cv::Size label_size = cv::getTextSize(pi.plate_no, cv::FONT_HERSHEY_SIMPLEX, 1, 3, &baseLine);

        int x = pi.bbox.xmin;
        int y = pi.bbox.ymin - label_size.height - baseLine;
        if (y < 0)
            y = 0;
        if (x + label_size.width > image.cols)
            x = image.cols - label_size.width;

        cv::rectangle(
                image,
                cv::Rect(cv::Point(x, y), cv::Size(label_size.width, label_size.height + baseLine)),
                cv::Scalar(255, 255, 255), cv::FILLED);

        uniText.PutText(image, pi.plate_no, cv::Point(x, y + label_size.height), cv::Scalar(0,255,0), false);
        LPR_Data = pi.plate_color + pi.plate_no;
    }
    //cv::imwrite("test.jpg", image);

    return image;
}

Mat PLR::test_mtcnn_plate(Mat img) {
    ncnn::Mat sample = ncnn::Mat::from_pixels(img.data, ncnn::Mat::PIXEL_BGR, img.cols, img.rows);
    std::vector<PlateInfo> objects;
    detector->plate_detect(sample, objects);
    lpr->decode_plate_infos(objects);
//    for (auto pi : objects) {
//        cout << "plate_no: " << pi.plate_color << pi.plate_no << " box:" << pi.bbox.xmin << ","
//             << pi.bbox.ymin << "," << pi.bbox.xmax << "," << pi.bbox.ymax << "," << pi.bbox.score << endl;
//    }

    return draw_plate_results(objects, img);
}
