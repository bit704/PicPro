#include "chromatic.h"
#include "ui_chromatic.h"

Chromatic::Chromatic(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Chromatic)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_QuitOnClose,false); //设置父窗口关闭后同时退出

}

Chromatic::~Chromatic()
{
    delete ui;
}

void Chromatic::showImage(QLabel* ql,Mat image,bool scaled) //在指定组件上展示图片
{
    Mat simage = image.clone();
    int channel=simage.channels();
    QImage img;
    if(channel==3)
    {
        for(int i=0;i<simage.rows;i++) //opencv为BGR,qimage为RGB,需要交换一下顺序
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

void Chromatic::on_openPic_clicked() //打开图片
{
    QString picFileName;
    picFileName = QFileDialog::getOpenFileName(this,"选择彩色图片",".","Images (*.png *.bmp *.jpg *.tif *.GIF )");
    if(picFileName.isEmpty())
    {
        return;
    }
    image=imread(std::string((const char *)picFileName.toLocal8Bit())); //这样才可以读取中文路径
    showImage(ui->showPic1,image,true);
    return;
}

void Chromatic::on_gray_clicked() //灰度化
{
    if(image.rows==0)
    {
        QMessageBox::warning(this,"提示","没有打开图片");
        return;
    }
    //根据人眼敏感度对RGB三分量进行加权平均
    Mat gimage = Mat(image.rows,image.cols,DataType<uchar>::type); //对应的灰度图像
    for(int i=0;i<gimage.rows;++i)
    {
        for(int j=0;j<gimage.cols;++j)
        {
            gimage.at<uchar>(i,j) =
                    (uchar)(image.at<Vec3b>(i,j)[0]*0.114+image.at<Vec3b>(i,j)[1]*0.587+image.at<Vec3b>(i,j)[2]*0.299);
            //opencv为BGR
            //f(i,j)=0.299*R(i,j)+0.587*G(i,j)+0.114*B(i,j)
        }
    }
    showImage(ui->showPic2,gimage,true);
    return;
}


void Chromatic::on_convert_clicked() //24位真彩色图像转换为256色彩色图像
{
    if(image.rows==0)
    {
        QMessageBox::warning(this,"提示","没有打开图片");
        return;
    }
    Mat cimage=image.clone();
    //使用流行色算法
    vector<int> color(4096);
    //取R，G，B的最高四位,一共4096种颜色,统计在图片中的出现次数
    for(int i=0;i<image.rows;++i)
    {
        for(int j=0;j<image.cols;++j)
        {
            int c = (image.at<Vec3b>(i,j)[0]/16)*256+(image.at<Vec3b>(i,j)[1]/16)*16+(image.at<Vec3b>(i,j)[2]/16);
            //将BGR颜色映射到一维数组上，对应0-4095中的一个数字
            color[c]++;
        }
    }
    map<int,int,greater<int>> table; //对键排序
    for(int i=0;i<4096;i++) table[color[i]] = i;
    //建立出现次数和颜色的对应关系，从大到小排序，取出现次数最多的256种颜色作为高频256色

    for(int i=0;i<cimage.rows;++i)
    { 
        for(int j=0;j<cimage.cols;++j)
        {
            double mind = numeric_limits<double>::max();
            int minc = 0;
            //计算该颜色与高频256色之间的距离，取距离最小的高频颜色代表该颜色
            map<int,int>::iterator iter=table.begin();
            for(int k=0; k<256 ;iter++,k++)
            {
                //计算距离
                int c = (*iter).second;
                double distance=
                        sqrt(pow(cimage.at<Vec3b>(i,j)[0]/16-c/256,2)+
                            pow(cimage.at<Vec3b>(i,j)[1]/16-c%256/16,2)+
                            pow(cimage.at<Vec3b>(i,j)[2]/16-c%256%16,2)
                        );
                //更新最小值
                if(distance<mind)
                {
                    mind = distance;
                    minc = c;
                }
            }
            //颜色代换
            cimage.at<Vec3b>(i,j)[0] = minc/16; // /256*16
            cimage.at<Vec3b>(i,j)[1] = minc%256; // %256/16*16
            cimage.at<Vec3b>(i,j)[2] = minc%256%16*16;  // %256%16*16
        }
    }
    showImage(ui->showPic2,cimage,true);
    return;
}
