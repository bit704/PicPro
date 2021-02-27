#ifndef PEDESTRIAN_H
#define PEDESTRIAN_H

#include <QWidget>
#include <stdio.h>
#include <iostream>
#include <math.h>
#include <QFileDialog>
#include <opencv2/opencv.hpp>
#include <opencv2/video/background_segm.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/ml/ml.hpp>

#include <QDebug>

using namespace cv;
using namespace std;

namespace Ui {
class Pedestrian;
}

class Pedestrian : public QWidget
{
    Q_OBJECT

public:
    explicit Pedestrian(QWidget *parent = 0);
    ~Pedestrian();

private slots:

    void on_loadvideo_clicked();

private:
    Ui::Pedestrian *ui;

};

#endif // PEDESTRIAN_H
