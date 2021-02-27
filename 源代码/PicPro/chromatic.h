#ifndef CHROMATIC_H
#define CHROMATIC_H

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
class Chromatic;
}

class Chromatic : public QWidget
{
    Q_OBJECT

public:
    explicit Chromatic(QWidget *parent = 0);
    ~Chromatic();

private slots:
    void on_openPic_clicked();

    void on_gray_clicked();

    void on_convert_clicked();

private:
    Ui::Chromatic *ui;
    Mat image;

    void showImage(QLabel* ql,Mat image,bool scaled=false);
};

#endif // CHROMATIC_H
