#include "minus.h"
#include "ui_minus.h"

Minus::Minus(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Minus)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_QuitOnClose,false); //设置父窗口关闭后同时退出
}

Minus::~Minus()
{
    delete ui;
}

void Minus::showImage(QLabel* ql,Mat simage,bool scaled=false) //在指定组件上展示图片
{
    int channel=simage.channels();
    QImage img;
    if(channel==3)
    {
        for(int i=0;i<simage.rows;i++) //qimage为RGB,需要交换一下顺序
        {
            for(int j=0;j<simage.cols;j++)
            {
                uchar b=simage.at<Vec3b>(i,j)[0];
                uchar r=simage.at<Vec3b>(i,j)[2];
                simage.at<Vec3b>(i,j)[0]=r;
                simage.at<Vec3b>(i,j)[2]=b;
            }
        }
        img = QImage((const unsigned char*)(simage.data),simage.cols,simage.rows,simage.cols*simage.channels(),QImage::Format_RGB888);
    }
    else if(channel==4) img = QImage((const unsigned char*)(simage.data),simage.cols, simage.rows ,simage.cols*simage.channels(), QImage::Format_ARGB32);
    else img = QImage((const unsigned char*)(simage.data),simage.cols, simage.rows , simage.cols*simage.channels(),QImage::Format_Indexed8);
    //第四个参数防止图片数据没有按照4字节对齐导致显示扭曲
    if(scaled) img=img.scaled(ql->size(),Qt::IgnoreAspectRatio,Qt::SmoothTransformation); //是否适应窗口长度
    ql->setPixmap(QPixmap::fromImage(img));
    return;
}

void Minus::on_pushButton_bg_clicked() //打开背景图
{
    QString picFileName;
    picFileName = QFileDialog::getOpenFileName(this,"选择图片",".","Images (*.png *.bmp *.jpg *.tif *.GIF )");
    if(picFileName.isEmpty())
    {
        return;
    }
    bgimage=imread(std::string((const char *)picFileName.toLocal8Bit())); //这样才可以读取中文路径
    Mat processedimage=Mat(bgimage.rows,bgimage.cols,DataType<uchar>::type);
    for(int i=0;i<bgimage.rows;i++)
    {
        for(int j=0;j<bgimage.cols;j++)
        {
            processedimage.at<uchar>(i,j) = bgimage.at<Vec3b>(i,j)[0];
        }
    }
    bgimage=processedimage.clone();
    showImage(ui->showPic1,bgimage,true);
    return;
}

void Minus::on_pushButton_bg_obj_clicked() //打开含目标对象图
{
    QString picFileName;
    picFileName = QFileDialog::getOpenFileName(this,"选择图片",".","Images (*.png *.bmp *.jpg *.tif *.GIF )");
    if(picFileName.isEmpty())
    {
        return;
    }
    bg_objimage=imread(std::string((const char *)picFileName.toLocal8Bit()));
    Mat processedimage=Mat(bg_objimage.rows,bg_objimage.cols,DataType<uchar>::type);
    for(int i=0;i<bg_objimage.rows;i++)
    {
        for(int j=0;j<bg_objimage.cols;j++)
        {
            processedimage.at<uchar>(i,j) = bg_objimage.at<Vec3b>(i,j)[0];
        }
    }
    bg_objimage=processedimage.clone();
    showImage(ui->showPic2,bg_objimage,true);
    return;
}


void Minus::on_pushButton_obj_clicked() //相减得到目标对象图并显示
{
    if(bgimage.cols==0)
    {
        QMessageBox::warning(this,"提示","没有打开背景图片");
        return;
    }
    if(bg_objimage.cols==0)
    {
        QMessageBox::warning(this,"提示","没有打开含目标对象图片");
        return;
    }
    double similarity=0,diffnum=0;
    for(int i=0;i<bgimage.rows;i++) //求相似度
    {
        for(int j=0;j<bgimage.cols;j++)
        {
            if(abs((int)bgimage.at<uchar>(i,j) - (int)bg_objimage.at<uchar>(i,j))>10) diffnum++;
        }
    }
    similarity = 1-diffnum/(bgimage.rows*bgimage.cols);
    //qDebug()<<similarity<<endl;
    while(similarity<0.7)
    {
        for(int i=0;i<bgimage.rows;i++) //改变灰度
        {
            for(int j=0;j<bgimage.cols;j++)
            {
                bgimage.at<uchar>(i,j) *= 1.1;
            }
        }
        diffnum=0;
        for(int i=0;i<bgimage.rows;i++) //求相似度
        {
            for(int j=0;j<bgimage.cols;j++)
            {
                if(abs((int)bgimage.at<uchar>(i,j) - (int)bg_objimage.at<uchar>(i,j))>10) diffnum++;
            }
        }
        similarity = 1-diffnum/(bgimage.rows*bgimage.cols);
    }
    objimage=bgimage.clone();
    for(int i=0;i<objimage.rows;i++) //相减，找出目标部分
    {
        for(int j=0;j<objimage.cols;j++)
        {
            if(abs((int)bgimage.at<uchar>(i,j) - (int)bg_objimage.at<uchar>(i,j))<=50)
            objimage.at<uchar>(i,j) = 0;
            else
            {
                objimage.at<uchar>(i,j) = bg_objimage.at<uchar>(i,j);
            }

        }
    }
    for(int i=0;i<objimage.rows;i++) //扩展低灰度部分
    {
        for(int j=0;j<objimage.cols;j++)
        {
            objimage.at<uchar>(i,j) = (uchar)round(255 * pow(objimage.at<uchar>(i,j)/255.0,0.4));
        }
    }
    showImage(ui->showPic3,objimage,true);
    return;
}
