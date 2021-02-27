#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->sampling,SIGNAL(currentIndexChanged(QString)),this,SLOT(samplingChanged(QString)));
    connect(ui->quantificat,SIGNAL(currentIndexChanged(QString)),this,SLOT(quantificatChanged(QString)));
    connect(ui->pointop,SIGNAL(currentIndexChanged(QString)),this,SLOT(pointopChanged(QString)));

    ui->grayhistogram1->xAxis->setRange(0,256); //初始化直方图横纵坐标
    ui->grayhistogram2->xAxis->setRange(0,256);
    ui->grayhistogram1->yAxis->setRange(0,2500);
    ui->grayhistogram2->yAxis->setRange(0,2500);

    ui->scale->setSuffix("%");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showImage(QLabel* ql,QCustomPlot* qcp, QTextBrowser* qtb,Mat image,bool scaled=false) //在指定组件上展示图片、直方图、信息
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
    paintGrayHistogram(qcp,qtb,simage);
    on_threshold_valueChanged(ui->threshold->value());
    return;
}

void MainWindow::recover() //将处理后的图片恢复为原图
{
    processedimage=Mat(image.rows,image.cols,DataType<uchar>::type);
    for(int i=0;i<image.rows;i++)
    {
        for(int j=0;j<image.cols;j++)
        {
            processedimage.at<uchar>(i,j) = image.at<Vec3b>(i,j)[0];
            //如果原图是三通道灰度图，三个通道的值是一样的。
        }
    }
}


void MainWindow::on_actionOpenPic_triggered() //打开图片
{
    QString picFileName;
    picFileName = QFileDialog::getOpenFileName(this,"选择图片",".","Images (*.png *.bmp *.jpg *.tif *.GIF )");
    if(picFileName.isEmpty())
    {
        return;
    }
    image=imread(std::string((const char *)picFileName.toLocal8Bit())); //这样才可以读取中文路径
    processedimage=Mat(image.rows,image.cols,DataType<uchar>::type);  //对应的灰度图
    for(int i=0;i<image.rows;i++)
    {
        for(int j=0;j<image.cols;j++)
        {
            processedimage.at<uchar>(i,j) = image.at<Vec3b>(i,j)[0];
            //如果原图是三通道灰度图，三个通道的值是一样的。
        }
    }
    showImage(ui->showPic1,ui->grayhistogram1,ui->information1,image,true);
    showImage(ui->showPic2,ui->grayhistogram2,ui->information2,image);
    return;
}

void MainWindow::samplingChanged(QString text) //采样点数改变
{
    if(image.rows==0)
    {
        QMessageBox::warning(this,"提示","没有打开图片");
        ui->sampling->setCurrentText("原图");
        return;
    }
    recover();
    if(text!="原图")
    {
        int num;
        num = text.toInt();
        if(num>processedimage.cols || num>processedimage.rows)
        {
            QMessageBox::warning(this,"提示","当前图片尺寸过小");
            return;
        }
        int sc=processedimage.cols/num,sr=processedimage.rows/num; //采样块长宽
        for(int i=0;i<num;i++)
        {
            for(int j=0;j<num;j++)
            {
                int tot=0;
                for(int ni=i*sr; ni<(i+1)*sr && ni<processedimage.rows; ni++)
                {
                    for(int nj=j*sc; nj<(j+1)*sc && ni<processedimage.cols; nj++)
                    {
                        tot += (int)processedimage.at<uchar>(ni,nj);  //统计采样块内的总灰度值
                    }
                }
                uchar newpixel=(uchar)(tot/(sc*sr)); //计算平均灰度值
                for(int ni=i*sr; ni<(i+1)*sr && ni<processedimage.rows; ni++)
                {
                    for(int nj=j*sc; nj<(j+1)*sc && ni<processedimage.cols; nj++)
                    {
                        processedimage.at<uchar>(ni,nj) = newpixel; //将采样块内的点灰度值全部更新为平均灰度值
                    }
                }
            }
        }
    }
    showImage(ui->showPic2,ui->grayhistogram2,ui->information2,processedimage);
    return;
}

void MainWindow::quantificatChanged(QString text) //量化等级改变
{
    if(image.rows==0)
    {
        QMessageBox::warning(this,"提示","没有打开图片");
        ui->quantificat->setCurrentText("原图");
        return;
    }
    recover();
    if(text!="原图")
    {
        int level;
        level = text.toInt(); //量化等级
        for(int i=0;i<processedimage.rows;i++)
        {
            for(int j=0;j<processedimage.cols;j++)
            {
                processedimage.at<uchar>(i,j) = processedimage.at<uchar>(i,j)/(256/level)*(256/level);
                //根据量化等级对灰度值进行调整，一级为256/level
            }
        }
    }
    showImage(ui->showPic2,ui->grayhistogram2,ui->information2,processedimage);
    return;
}

void MainWindow::on_actionSavePic_triggered() //保存图片
{
    QString picFileName;
    picFileName = QFileDialog::getSaveFileName(this,"保存图片",".","Images (*.png *.bmp *.jpg *.tif *.GIF )");
    if(picFileName.isEmpty())
    {
        return;
    }
    image=processedimage.clone(); //此时原图也更改为处理后的图片
    showImage(ui->showPic1,ui->grayhistogram1,ui->information1,image,true);
    showImage(ui->showPic2,ui->grayhistogram2,ui->information2,image);
    imwrite(std::string((const char *)picFileName.toLocal8Bit()),image);
    return;
}

void MainWindow::on_eight_clicked() //生成8幅位平面表示的二值图
{
    if(image.rows==0)
    {
        QMessageBox::warning(this,"提示","没有打开图片");
        ui->sampling->setCurrentText("原图");
        return;
    }
    Mat eightimage=Mat(image.rows,image.cols*8,DataType<uchar>::type);
    for(int k=0;k<8;k++)
    {
        for(int i=0;i<image.rows;i++)
        {
            for(int j=0;j<image.cols;j++)
            {
                uchar x=image.at<Vec3b>(i,j)[0];
                uchar y=pow(2,k);
                if((x&y) != 0)  //利用位运算进行切割
                eightimage.at<uchar>(i,j+image.cols*k) = 255;
                else
                eightimage.at<uchar>(i,j+image.cols*k) = 0;
            }
        }
    }
    Mat neightimage(200,1600,eightimage.type()); //将八幅图拼接为一长条
    cv::resize(eightimage,neightimage,neightimage.size(),0,0,INTER_LINEAR); //缩小
    //这里的size后面必须加()
    namedWindow("二值图",WINDOW_AUTOSIZE);
    imshow("二值图",neightimage);
    waitKey(0);
    destroyAllWindows();
    return;
}

void MainWindow::paintGrayHistogram(QCustomPlot* qcp,QTextBrowser* inf,Mat pimage) //绘制灰度直方图,显示图像信息
{
    qcp->clearPlottables();
    QVector<double> datax;
    QVector<double> datay;
    for(int i=0;i<256;i++)
    {
        datax.push_back(i);
        datay.push_back(0); //初始化x y坐标
    }
    int totg=0;
    for(int i=0;i<pimage.rows;i++)
    {
        for(int j=0;j<pimage.cols;j++)
        {
            uchar g;
            if(pimage.channels()==3) g=pimage.at<Vec3b>(i,j)[0];
            else if(pimage.channels()==1) g=pimage.at<uchar>(i,j);
            totg += g;
            datay[g]++;  //统计灰度值
        }
    }
    int num=pimage.rows*pimage.cols; //像素总数
    int avg=totg/num; //平均灰度
    int nownum=0;
    int median; //中值灰度
    for(int i=0;i<256;i++)
    {
        nownum += datay[i];
        if(nownum>=num/2) //找到中值灰度
        {
            median=i;
            break;
        }
    }
    double std=0; //标准差
    for(int i=0;i<256;i++)
    {
        std += pow(datay[i]-avg,2);
    }
    std = sqrt(std/num);
    QString information;
    information.sprintf("直方图信息\n像素总数:%d\n平均灰度:%d\n中值灰度:%d\n标准差:%.2f\n",num,avg,median,std);
    inf->setText(information);
    int yrange=0;
    for(int i=0;i<256;i++)
    {
        if(datay[i]>yrange) yrange=datay[i]; //找出最大值作为坐标轴上限
    }
    qcp->xAxis->setRange(0,256);
    qcp->yAxis->setRange(0,(int)yrange);
    QCPBars *bars = new QCPBars( qcp->xAxis,  qcp->yAxis);
    bars->setData(datax, datay); //根据数据进行绘图
    bars->setPen(QColor(0, 0, 0));
    bars->setWidth(0.05);
    qcp->setVisible(true);
    qcp->replot();
    return;
}

void MainWindow::on_threshold_valueChanged(int value) //阈值改变
{
    Mat timage=processedimage.clone();
    for(int i=0;i<timage.rows;i++)
    {
        for(int j=0;j<timage.cols;j++)
        {
            uchar g;
            if(timage.channels()==3) g=timage.at<Vec3b>(i,j)[0];
            else if(timage.channels()==1) g=timage.at<uchar>(i,j);
            if(g>value) g=255;
            else g=0;
            if(timage.channels()==3) timage.at<Vec3b>(i,j)=Vec3b(g,g,g);
            else if(timage.channels()==1) timage.at<uchar>(i,j)=g;
        }
    }
    QImage img;
    img = QImage((const unsigned char*)(timage.data),timage.cols, timage.rows , timage.cols*timage.channels(),QImage::Format_Indexed8);
    img=img.scaled(ui->showPic3->size(),Qt::IgnoreAspectRatio,Qt::SmoothTransformation); //适应窗口长度
    ui->showPic3->setPixmap(QPixmap::fromImage(img));
    return;
}

void MainWindow::on_equilibrium_clicked() //传统直方图均衡化
{
    if(image.rows==0)
    {
        QMessageBox::warning(this,"提示","没有打开图片");
        ui->sampling->setCurrentText("原图");
        return;
    }
    recover();
    double p[256];
    for(int i=0;i<256;i++) p[i]=0;
    for(int i=0;i<processedimage.rows;i++)
    {
        for(int j=0;j<processedimage.cols;j++)
        {
            p[processedimage.at<uchar>(i,j)]++;
        }
    }
    for(int i=0;i<256;i++) p[i] /= (processedimage.cols*processedimage.rows); //灰度分布概率
    for(int i=1;i<256;i++) p[i] += p[i-1];
    for(int i=0;i<256;i++) p[i] = round(p[i]*255);  //均衡化，将老灰度级映射到新的灰度级
    for(int i=0;i<processedimage.rows;i++)
    {
        for(int j=0;j<processedimage.cols;j++)
        {
            processedimage.at<uchar>(i,j)=(uchar)p[processedimage.at<uchar>(i,j)]; //更新灰度级
        }
    }
    showImage(ui->showPic2,ui->grayhistogram2,ui->information2,processedimage);
    return;
}

void MainWindow::on_equilibriumBBHE_clicked() //BBHE直方图均衡化
{
    if(image.rows==0)
    {
        QMessageBox::warning(this,"提示","没有打开图片");
        ui->sampling->setCurrentText("原图");
        return;
    }
    recover();
    double p[256];
    for(int i=0;i<256;i++) p[i]=0;
    for(int i=0;i<processedimage.rows;i++)
    {
        for(int j=0;j<processedimage.cols;j++)
        {
            p[processedimage.at<uchar>(i,j)]++;
        }
    }
    int mean=0; //平均灰度值
    for(int i=0;i<256;i++) mean += (int)p[i]*i;
    mean = round((double)mean/(processedimage.cols*processedimage.rows));

    double x1[256],x2[256]; //分解为两个子图像
    int totx1=0,totx2=0;
    for(int i=0;i<mean;i++)
    {
        x1[i]=p[i];
        totx1+=x1[i];
    }
    for(int i=0;i<mean;i++) x1[i] /= totx1; //灰度分布概率
    for(int i=1;i<mean;i++) x1[i] += x1[i-1];  //i的初值为1
    for(int i=0;i<mean;i++) x1[i] = round(x1[i]*mean); //均衡化，将老灰度级映射到新的灰度级
    for(int i=mean;i<256;i++)
    {
        x2[i]=p[i];
        totx2+=x2[i];
    }
    for(int i=mean;i<256;i++) x2[i] /= totx2; //灰度分布概率
    for(int i=mean+1;i<256;i++) x2[i] += x2[i-1];
    for(int i=mean;i<256;i++) x2[i] = round(x2[i]*(255-mean))+mean; //均衡化，将老灰度级映射到新的灰度级
    for(int i=0;i<mean;i++) p[i]=x1[i];
    for(int i=mean;i<256;i++) p[i]=x2[i]; //合并子图为原图
    for(int i=0;i<processedimage.rows;i++)
    {
        for(int j=0;j<processedimage.cols;j++)
        {
            processedimage.at<uchar>(i,j)=(uchar)p[processedimage.at<uchar>(i,j)]; //更新灰度级
        }
    }
    showImage(ui->showPic2,ui->grayhistogram2,ui->information2,processedimage);
    return;
}

void MainWindow::pointopChanged(QString text) //点运算
{
    if(image.rows==0)
    {
        QMessageBox::warning(this,"提示","没有打开图片");
        ui->quantificat->setCurrentText("原图");
        return;
    }
    recover();
    if(text!="原图")
    {

        for(int i=0;i<processedimage.rows;i++)
        {
            for(int j=0;j<processedimage.cols;j++)
            {
                if(text=="负片变换")
                processedimage.at<uchar>(i,j) = 255-processedimage.at<uchar>(i,j);
                if(text=="对数变换") //对数变换主要用于将图像的低灰度值部分扩展，将其高灰度值部分压缩，以达到强调图像低灰度部分的目的。
                processedimage.at<uchar>(i,j) = (uchar)round(255 * log(processedimage.at<uchar>(i,j)/255.0*10+1)/log(11));
                if(text=="伽马变换") // 伽马变换主要用于图像的校正，将漂白的图片或者是过黑的图片，进行修正
                processedimage.at<uchar>(i,j) = (uchar)round(255 * pow(processedimage.at<uchar>(i,j)/255.0,5));
                if(text=="对比度拉伸") //增强对比度
                {
                    uchar g=processedimage.at<uchar>(i,j);
                    if(g<100) g=g/2;
                    if(g>=100 && g<=150) g=(g-100)*2+50;
                    if(g>150) g=(g-150)+150;
                    processedimage.at<uchar>(i,j)=g;

                }
            }
        }
    }
    showImage(ui->showPic2,ui->grayhistogram2,ui->information2,processedimage);
    return;
}

void MainWindow::on_rotate_sliderMoved(int angle) //图像旋转
{
    if(image.rows==0)
    {
        QMessageBox::warning(this,"提示","没有打开图片");
        ui->quantificat->setCurrentText("原图");
        return;
    }
    recover();
    ui->degree->setText(QString::number(angle)+"度");

    double pi=3.141;
    double radian = (float) (angle /180.0 * pi); //弧度

    double sinr=abs(sin(radian));
    double cosr=abs(cos(radian));
    int newcols = (int) (processedimage.cols * cosr +processedimage.rows * sinr); //旋转后新宽度
    int newrows = (int) (processedimage.cols * sinr + processedimage.rows * cosr); //旋转后新高度

    int border =(int) (max(processedimage.cols, processedimage.rows)* 1.5);
    int nx = (border - processedimage.cols)/2;
    int ny = (border - processedimage.rows)/2;
    copyMakeBorder(processedimage,processedimage,ny,ny,nx,nx, 0); //给图像加上多余边框
    Point2f center((float)processedimage.cols/2,(float)processedimage.rows/2); //旋转中心
    Mat M=getRotationMatrix2D(center,-angle,1.0); //旋转矩阵

    warpAffine(processedimage,processedimage,M,processedimage.size());
//    Rect r = Rect((processedimage.cols-newcols)/2,(processedimage.rows-newrows)/2,newcols,newrows);
//    processedimage = Mat(processedimage,r); //裁剪多余边框
    //裁剪之后Qt显示会出问题，目前没有解决，所以没有裁剪

    showImage(ui->showPic2,ui->grayhistogram2,ui->information2,processedimage,true);
}


void MainWindow::on_scale_valueChanged(double scale) //缩放
{
    if(image.rows==0)
    {
        QMessageBox::warning(this,"提示","没有打开图片");
        ui->quantificat->setCurrentText("原图");
        return;
    }
    recover();
    int newrows = (int)processedimage.rows*scale/100;
    int newcols = (int)processedimage.cols*scale/100;
    Mat newimage = Mat::zeros(newrows,newcols,DataType<uchar>::type); //初始化为0
    QString insertMethod=ui->insert->currentText();
    for(int i=0;i<newimage.rows;i++)
    {
        for(int j=0;j<newimage.cols;j++)
        {
            if(newimage.at<uchar>(i,j)==0)
            {
                if(insertMethod=="最近邻插值")
                {
                    int ni = (int)i*100/scale; //新坐标在原图对应的坐标
                    int nj = (int)j*100/scale;
                    newimage.at<uchar>(i,j) = processedimage.at<uchar>(ni,nj);
                }
                if(insertMethod=="双线性插值")
                {
                    double ni = i*100/scale;
                    double nj = j*100/scale;
                    int ri = ni;
                    int rj = nj;
                    double u=ni-ri;
                    double v=nj-rj;\
                    if(ri+1>=processedimage.rows || rj+1>=processedimage.cols) //后面的计算中会越界
                    {
                        newimage.at<uchar>(i,j) = processedimage.at<uchar>(ri,rj);
                    }
                    else
                    {
                        uchar x1=processedimage.at<uchar>(ri,rj);
                        uchar x2=processedimage.at<uchar>(ri,rj+1);
                        uchar x3=processedimage.at<uchar>(ri+1,rj);
                        uchar x4=processedimage.at<uchar>(ri+1,rj+1);
                        newimage.at<uchar>(i,j) = (uchar)((1-u)*(1-v)*x1+(1-u)*v*x2+u*(1-v)*x3+u*v*x4); //双线性插值
                    }
                }

            }
        }
    }
    processedimage=newimage.clone();
    showImage(ui->showPic2,ui->grayhistogram2,ui->information2,processedimage);
}

void MainWindow::on_horizontalmove_sliderMoved(int position) //水平移动
{
    if(image.rows==0)
    {
        QMessageBox::warning(this,"提示","没有打开图片");
        ui->quantificat->setCurrentText("原图");
        return;
    }
    recover();
    Mat newimage = Mat::zeros(processedimage.rows,processedimage.cols,DataType<uchar>::type); //初始化为0
    for(int i=0;i<processedimage.rows;i++)
    {
        for(int j=0;j<processedimage.cols;j++)
        {
            int nj = j+(int)processedimage.cols*position/100.0; //移动位置
            if(nj>=0 && nj<newimage.cols) newimage.at<uchar>(i,nj) = processedimage.at<uchar>(i,j);
        }
    }
    processedimage=newimage.clone();
    showImage(ui->showPic2,ui->grayhistogram2,ui->information2,processedimage);
}

void MainWindow::on_verticalmove_sliderMoved(int position) //垂直移动
{
    if(image.rows==0)
    {
        QMessageBox::warning(this,"提示","没有打开图片");
        ui->quantificat->setCurrentText("原图");
        return;
    }
    recover();
    Mat newimage = Mat::zeros(processedimage.rows,processedimage.cols,DataType<uchar>::type); //初始化为0
    for(int i=0;i<processedimage.rows;i++)
    {
        for(int j=0;j<processedimage.cols;j++)
        {
            int ni = i+(int)processedimage.rows*position/100.0;
            if(ni>=0 && ni<newimage.rows) newimage.at<uchar>(ni,j) = processedimage.at<uchar>(i,j);
        }
    }
    processedimage=newimage.clone();
    showImage(ui->showPic2,ui->grayhistogram2,ui->information2,processedimage);
}


void MainWindow::on_minus_clicked() //图像相减
{
    minus = new Minus();
    minus->show();
}

void MainWindow::on_bmp2txt_clicked() //bmp2txt
{
    if(image.rows==0)
    {
        QMessageBox::warning(this,"提示","没有打开图片");
        ui->sampling->setCurrentText("原图");
        return;
    }
    bmp2txt = new Bmp2txt(image);
    bmp2txt->show();
}

void MainWindow::on_pedestrain_clicked() //行人分析
{
    pedestrian = new Pedestrian();
    pedestrian->show();
}

void MainWindow::on_transform_clicked() //图片变形
{
    transform = new TransForm();
    transform->show();
}

void MainWindow::on_chromatic_clicked() //彩色图像处理
{
    chromatic = new Chromatic();
    chromatic->show();
}

void MainWindow::on_convert_clicked() //图像变换
{
    convert =new Convert();
    convert->show();
}

void MainWindow::on_enhance_clicked() //图像增强
{
    enhance = new Enhance();
    enhance->show();
}

void MainWindow::on_segmentation_clicked() //图像分割
{
    segmentation = new Segmentation();
    segmentation->show();
}

void MainWindow::on_compress_clicked() //图像编码与压缩
{
    compress = new Compress();
    compress->show();
}
