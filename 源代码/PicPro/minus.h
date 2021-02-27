#ifndef MINUS_H
#define MINUS_H

#include <QWidget>
#include <QFileDialog>
#include <QLabel>
#include <QImage>
#include <QMessageBox>
#include <QDebug>
#include <opencv2/opencv.hpp>
using namespace cv;

namespace Ui {
class Minus;
}

class Minus : public QWidget
{
    Q_OBJECT

public:
    explicit Minus(QWidget *parent = 0);
    ~Minus();

private slots:
    void on_pushButton_bg_clicked();

    void on_pushButton_bg_obj_clicked();

    void on_pushButton_obj_clicked();

private:
    Ui::Minus *ui;

    Mat bgimage; //背景图
    Mat bg_objimage;   //含目标对象图
    Mat objimage; //目标对象图

    void showImage(QLabel* ,Mat ,bool);
};

#endif // MINUS_H
