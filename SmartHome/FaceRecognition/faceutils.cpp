#include "faceutils.h"

FaceUtils::CGarbo FaceUtils::carbo;
FaceUtils *FaceUtils::faceUtils = NULL;

FaceUtils::FaceUtils()
{
    eigenFaceRecognizer = EigenFaceRecognizer::create();
//    fisherFaceRecognizer = FisherFaceRecognizer::create();
    LBPHFaceRecognizer = LBPHFaceRecognizer::create();

    isFaceTrained = false;
    faceTrainThread = new FaceUtils::FaceTrain(this);
}

FaceUtils::~FaceUtils()
{
    delete faceTrainThread;
}

FaceUtils *FaceUtils::getInstance()
{
    if(FaceUtils::faceUtils == NULL)
        FaceUtils::faceUtils = new FaceUtils();
    return FaceUtils::faceUtils;
}


vector<Rect> FaceUtils::faceDetection(const Mat &image)
{
    vector<Rect> faces;

    if(cascadeClassifier.empty() && !cascadeClassifier.load(FACE_DETECTION_XML_PATH))
    {
        qDebug("文件加载失败");
    }else{

        Mat faceDetectionGray;
        cvtColor(image, faceDetectionGray, COLOR_BGR2GRAY);
        equalizeHist(faceDetectionGray, faceDetectionGray);
        cascadeClassifier.detectMultiScale(faceDetectionGray, faces,
            1.1, 3, 0
            //|CASCADE_FIND_BIGGEST_OBJECT
            //|CASCADE_DO_ROUGH_SEARCH
            |CASCADE_SCALE_IMAGE,
            Size(30, 30));
    }

    return faces;
}

int FaceUtils::faceRecognition(const Mat &image, RecognizerModel recognizerModel)
{
    if(!this->isFaceTrained)
    {
        qDebug("请先进行人脸识别训练");
        return -1;
    }
    try
    {
        Mat faceRecognizerGray;
        cvtColor(image, faceRecognizerGray, COLOR_BGR2GRAY);

        int pridicted_label = -1;
        double predicted_confidence = 0.0;
        int temp = -1;

        switch(recognizerModel)
        {
        case PCA_MODEL:
            temp = eigenFaceRecognizer->predict(faceRecognizerGray);
            if(0 < temp && temp < 6){
                eigenFaceRecognizer->predict(faceRecognizerGray, pridicted_label, predicted_confidence);
                qDebug("faces_label:%d,confidence:%f",pridicted_label,predicted_confidence);
                if(predicted_confidence < 5000)
                        return temp;
                return -1;
            }else
                return temp;
        case FISHER_MODEL:
//            fisherFaceRecognizer->predict(faceRecognizerGray, pridicted_label, predicted_confidence);
//            qDebug("faces_label:%d,confidence:%f",pridicted_label,predicted_confidence);
//            return fisherFaceRecognizer->predict(faceRecognizerGray);
        case LBPH_MODEL:
            temp = LBPHFaceRecognizer->predict(faceRecognizerGray);
            if(0 < temp && temp < 6){
                LBPHFaceRecognizer->predict(faceRecognizerGray, pridicted_label, predicted_confidence);
                qDebug("faces_label:%d,confidence:%f",pridicted_label,predicted_confidence);
                if(predicted_confidence < 100)
                    return temp;
                return -1;
            }else
                return temp;
        default:
            return -1;
        }
    }
    catch (cv::Exception& e)
    {
        qDebug("人脸识别出错");
        cout << e.msg << endl;
        return -1;
    }
}

void FaceUtils::startTrainFace()
{
    emit(startTrain());
    if(!this->isFaceTrained)
    {
        this->isFaceTrained = this->loadFaceTrainFile();
        if(this->isFaceTrained)
        {
            emit(finishTrain(true));
            return;
        }
    }
    faceTrainThread->start();
}

bool FaceUtils::faceTrain()
{
    vector<Mat> images;
    vector<int> labels;

    try
    {
        if(!readCsvFile(FACE_RECOGNITION_CSV_PATH, images, labels))
            return false;
    }
    catch (cv::Exception& e)
    {
        qDebug("人脸识别训练文件加载出错");
        cout << e.msg << endl;
        return false;
    }

    if (images.size() <= 1)
    {
        qDebug("人脸识别训练样本为空");
        return false;
    }

    // 下面几行创建了一个特征脸模型用于人脸识别，
    // 通过CSV文件读取的图像和标签训练它。
    // T这里是一个完整的PCA变换
    // 如果你只想保留10个主成分，使用如下代码

    //      cv::createEigenFaceRecognizer(10);
    //
    // 如果你还希望使用置信度阈值来初始化，使用以下语句：
    //      cv::createEigenFaceRecognizer(10, 123.0);
    //
    // 如果你使用所有特征并且使用一个阈值，使用以下语句：
    //      cv::createEigenFaceRecognizer(0, 123.0);
    eigenFaceRecognizer = EigenFaceRecognizer::create(80);
    LBPHFaceRecognizer = LBPHFaceRecognizer::create(1,8,8,8);
    try
    {
        eigenFaceRecognizer->train(images, labels);
//        fisherFaceRecognizer->train(images, labels);
        LBPHFaceRecognizer->train(images, labels);
        eigenFaceRecognizer->save(FACE_PCA_MODEL_XML_PATH);
//        fisherFaceRecognizer->save(FACE_FISHER_MODEL_XML_PATH);
        LBPHFaceRecognizer->save(FACE_LBPH_MODEL_XML_PATH);
    }
    catch (cv::Exception& e)
    {
        qDebug("人脸识别训练出错");
        cout << e.msg << endl;
        return false;
    }

    return true;
}

bool FaceUtils::loadFaceTrainFile()
{
    try
    {
        eigenFaceRecognizer->read(FACE_PCA_MODEL_XML_PATH);
//        fisherFaceRecognizer->read(FACE_FISHER_MODEL_XML_PATH);
        LBPHFaceRecognizer->read(FACE_LBPH_MODEL_XML_PATH);
        return true;
    }
    catch(cv::Exception& e)
    {
        qDebug("人脸识别训练文件加载失败");
        cout << e.msg << endl;
        return false;
    }
}

bool FaceUtils::faceHasTrain()
{
    return this->isFaceTrained;
}

bool FaceUtils::readCsvFile(const string &filename, vector<Mat> &images, vector<int> &labels, char separator)
{
    std::ifstream file(filename.c_str(), ifstream::in);
    if (!file)
    {
        qDebug("人脸识别训练文件为空");
        return false;
    }

    string line, path, classlabel;
    while (getline(file, line))
    {
        stringstream liness(line);
        getline(liness, path, separator);
        getline(liness, classlabel);
        if(!path.empty() && !classlabel.empty())
        {
            images.push_back(imread(path, 0));
            labels.push_back(atoi(classlabel.c_str()));
        }
    }
    return true;
}
