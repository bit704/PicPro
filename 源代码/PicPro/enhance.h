#ifndef ENHANCE_H
#define ENHANCE_H

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
class Enhance;
}

class Enhance : public QWidget
{
    Q_OBJECT

public:
    explicit Enhance(QWidget *parent = 0);
    ~Enhance();

private slots:
    void on_openPic_clicked();

    void on_smooth_currentTextChanged(const QString&);

    void on_sharpen_currentTextChanged(const QString&);

    void on_convolution_clicked();

    void on_homo_clicked();

private:
    Ui::Enhance *ui;

    Mat image;
    Mat processedimage;

    void recover();

    void showImage(QLabel* ql,Mat image,bool scaled=false);

};

#endif // ENHANCE_H
