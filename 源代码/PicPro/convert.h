#ifndef CONVERT_H
#define CONVERT_H

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
class Convert;
}

class Convert : public QWidget
{
    Q_OBJECT

public:
    explicit Convert(QWidget *parent = 0);
    ~Convert();

private slots:
    void on_openPic_clicked();

    void on_spectrum_clicked();


    void on_lowpass_clicked();

    void on_highpass_clicked();

    void on_bandpass_clicked();

    void on_dct_clicked();

    void on_wt_clicked();
    
private:
    Ui::Convert *ui;

    Mat image;

    void showImage(QLabel* ql,Mat image,bool scaled=false);

    Mat gaussian_lowpass_filter(Mat &src, float sigma);
    Mat gaussian_highpass_filter(Mat &src, float sigma);
    Mat frequency_filter(Mat &padded,Mat &blur);

};

#endif // CONVERT_H
