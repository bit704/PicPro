#ifndef COMPRESS_H
#define COMPRESS_H

#include <QWidget>
#include <QLabel>
#include <QFileDialog>
#include <QImage>
#include <QMessageBox>
#include <QDebug>
#include <opencv2/opencv.hpp>
#include <fstream>

using namespace cv;
using namespace std;

namespace Ui {
class Compress;
}

class Compress : public QWidget
{
    Q_OBJECT

public:
    explicit Compress(QWidget *parent = 0);
    ~Compress();

private slots:

    void on_compress_clicked();

    void on_tree_clicked();
    
private:
    Ui::Compress *ui;

    Mat image;
    Mat processedimage;

};

#endif // COMPRESS_H
