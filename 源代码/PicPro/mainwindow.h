#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QImage>
#include <QDebug>
#include "qcustomplot.h"
#include "minus.h"
#include "bmp2txt.h"
#include "pedestrian.h"
#include "transform.h"
#include "chromatic.h"
#include "convert.h"
#include "enhance.h"
#include "segmentation.h"
#include "compress.h"

#include<opencv2/opencv.hpp>
using namespace cv;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_actionOpenPic_triggered();

    void samplingChanged(QString);  //采样点数改变

    void quantificatChanged(QString);  //量化级别改变

    void pointopChanged(QString);  //选择不同点运算

    void on_actionSavePic_triggered();

    void on_eight_clicked(); //8幅位平面二值图

    void on_threshold_valueChanged(int); //阈值改变

    void on_equilibrium_clicked(); //直方图均衡化

    void on_equilibriumBBHE_clicked(); //BBHE直方图均衡化

    void on_rotate_sliderMoved(int position);


    void on_scale_valueChanged(double arg1);

    void on_horizontalmove_sliderMoved(int position);

    void on_verticalmove_sliderMoved(int position);

    void on_minus_clicked();

    void on_bmp2txt_clicked();

    void on_pedestrain_clicked();

    void on_transform_clicked();

    void on_chromatic_clicked();

    void on_convert_clicked();

    void on_enhance_clicked();

    void on_segmentation_clicked();

    void on_compress_clicked();

private:
    Ui::MainWindow *ui;
    Mat image; //当前打开的图片
    Mat processedimage; //处理后的图片

    Minus* minus;
    Bmp2txt* bmp2txt;
    Pedestrian* pedestrian;
    TransForm* transform;
    Chromatic* chromatic;
    Convert* convert;
    Enhance* enhance;
    Segmentation* segmentation;
    Compress* compress;

    void recover();//将处理后的图片恢复为原图
    void showImage(QLabel*,QCustomPlot*, QTextBrowser*,Mat,bool); //展示图片
    void paintGrayHistogram(QCustomPlot*, QTextBrowser*,Mat);  //画直方图、显示信息


};

#endif // MAINWINDOW_H
