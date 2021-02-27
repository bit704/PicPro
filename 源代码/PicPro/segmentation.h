#ifndef SEGMENTATION_H
#define SEGMENTATION_H

#include <QWidget>
#include <QLabel>
#include <QFileDialog>
#include <QImage>
#include <QMessageBox>
#include <QDebug>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

namespace Ui {
class Segmentation;
}

class Segmentation : public QWidget
{
    Q_OBJECT

public:
    explicit Segmentation(QWidget *parent = 0);
    ~Segmentation();

private slots:
    void on_openPic_clicked();

    void on_detect_currentTextChanged(const QString &arg1);

    void on_close_clicked();

    void on_hough_clicked();

private:
    Ui::Segmentation *ui;

    Mat image;
    Mat processedimage;

    void recover();

    void showImage(QLabel* ql,Mat image,bool scaled=false);

    void CalMag(cv::Mat &src1,cv::Mat &src2,cv::Mat &dst1,cv::Mat &dst2);
    void connect(cv::Mat &src1,cv::Mat &src2,cv::Mat &dst);
    Mat rotate_arbitrarily_angle1(cv::Mat matSrc, float angle, bool direction,int height,int width);
    void fill(cv::Mat &src,cv::Mat &dst);

};

#endif // SEGMENTATION_H
