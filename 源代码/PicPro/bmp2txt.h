#ifndef BMP2TXT_H
#define BMP2TXT_H

#include <QWidget>

#include<opencv2/opencv.hpp>
using namespace cv;

namespace Ui {
class Bmp2txt;
}

class Bmp2txt : public QWidget
{
    Q_OBJECT

public:
    explicit Bmp2txt(QWidget *parent = 0);
    explicit Bmp2txt(Mat image,QWidget *parent = 0); //重写构造函数
    ~Bmp2txt();

private:
    Ui::Bmp2txt *ui;
};

#endif // BMP2TXT_H
