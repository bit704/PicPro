#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <QWidget>
#include <QFileDialog>
#include <QLabel>
#include <QImage>
#include <QMessageBox>
#include <QDebug>
#include <opencv2/opencv.hpp>

#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/render_face_detections.h>
#include <dlib/image_processing.h>
#include <dlib/gui_widgets.h>
#include <dlib/image_io.h>
#include <iostream>
#include<dlib/opencv/cv_image_abstract.h>
#include<vector>

using namespace cv;
using namespace dlib;
using namespace std;

namespace Ui {
class TransForm;
}

class TransForm : public QWidget
{
    Q_OBJECT

public:
    explicit TransForm(QWidget *parent = 0);
    ~TransForm();

private slots:
    void on_Apic_clicked();

    void on_Bpic_clicked();

    void on_show_clicked();

private:
    Ui::TransForm *ui;
    Mat Aimage;
    Mat Bimage;

    void showImage(QLabel* ql,Mat simage,bool scaled=false);

    QString ApicFileName;
    QString BpicFileName;

    struct correspondens{
        std::vector<int> index;
    };

    void faceLandmarkDetection(dlib::array2d<unsigned char>& img, shape_predictor sp, std::vector<Point2f>& landmark);
    void addKeypoints(std::vector<Point2f>& points,Size imgSize);
    void morpKeypoints(const std::vector<Point2f>& points1,const std::vector<Point2f>& points2,std::vector<Point2f>& pointsMorph, double alpha);
    void delaunayTriangulation(const std::vector<Point2f>& points1,const std::vector<Point2f>& points2,
                   std::vector<Point2f>& pointsMorph,double alpha,std::vector<correspondens>& delaunayTri,Size imgSize);
    void applyAffineTransform(Mat &warpImage, Mat &src, std::vector<Point2f> & srcTri, std::vector<Point2f> & dstTri);
    void morphTriangle(Mat &img1, Mat &img2, Mat &img, std::vector<Point2f> &t1, std::vector<Point2f> &t2, std::vector<Point2f> &t, double alpha);
    void morp(Mat &img1, Mat &img2, Mat& imgMorph, double alpha, const std::vector<Point2f> &points1, const std::vector<Point2f> &points2, const std::vector<correspondens> &triangle);
};

#endif // TRANSFORM_H
