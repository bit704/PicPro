#include "segmentation.h"
#include "ui_segmentation.h"

Segmentation::Segmentation(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Segmentation)
{
    ui->setupUi(this);
        setAttribute(Qt::WA_QuitOnClose,false); //设置父窗口关闭后同时退出
}

Segmentation::~Segmentation()
{
    delete ui;
}

void Segmentation::showImage(QLabel* ql,Mat image,bool scaled) //在指定组件上展示图片
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


void Segmentation::on_openPic_clicked() //打开图片
{
    QString picFileName;
    picFileName = QFileDialog::getOpenFileName(this,"选择图像",".","Images (*.png *.bmp *.jpg *.tif *.GIF )");
    if(picFileName.isEmpty())
    {
        return;
    }
    image=imread(std::string((const char *)picFileName.toLocal8Bit()),IMREAD_GRAYSCALE);
    //以灰度模式读入图片
    //这样才可以读取中文路径
    showImage(ui->showPic1,image,true);
    return;
}

void Segmentation::recover()
{
    processedimage=Mat(image.rows,image.cols,DataType<uchar>::type);
    for(int i=0;i<image.rows;i++)
    {
        for(int j=0;j<image.cols;j++)
        {
            processedimage.at<uchar>(i,j) = image.at<uchar>(i,j);
        }
    }
}



void Segmentation::on_detect_currentTextChanged(const QString& text) //边缘检测
{
    if(image.rows==0)
    {
        QMessageBox::warning(this,"提示","没有打开图片");
        ui->detect->setCurrentText("原图");
        return;
    }
    recover();
    if(text != "原图")
    {
        if(text == "拉普拉斯算子")
        {
            Mat kernel = (Mat_<char>(3,3) << -1, -1, -1,
                                             -1,  8, -1,
                                             -1, -1, -1);
            filter2D(image,processedimage,image.depth(),kernel);
        }
        else if(text == "Sobel算子")
        {
            Mat kernel = (Mat_<char>(3,3) << -1, -2, -1,
                                              0,  0,  0,
                                              1,  2,  1);
            filter2D(image,processedimage,image.depth(),kernel);
        }
        else if(text == "Prewitt算子")
        {
            Mat kernel = (Mat_<char>(3,3) << -1, -1, -1,
                                              0,  0,  0,
                                              1,  1,  1);
            filter2D(image,processedimage,image.depth(),kernel);
        }
    }
    showImage(ui->showPic2,processedimage,true);
}

void Segmentation::on_close_clicked() //边缘闭合
{
    if(image.rows==0)
    {
        QMessageBox::warning(this,"提示","没有打开图片");
        ui->detect->setCurrentText("原图");
        return;
    }
    Mat src = image.clone();
    Mat binary;
    Mat canny;
    Mat drawlines = Mat(image.rows,image.cols,CV_8UC3,Scalar::all(0)); //全黑
    cv::threshold(src, binary, 125, 255, cv::THRESH_BINARY); //二值化
    cv::Canny(binary, canny, 50, 125, 3); //边缘检测
    std::vector<cv::Vec4i> lines;
    HoughLinesP(canny,lines,1,CV_PI/180,50,0,50);

    for (size_t i = 0; i < lines.size(); i++)
    {
        cv::Vec4i& linex = lines[i];
        line(drawlines, cv::Point(linex[0], linex[1]), cv::Point(linex[2], linex[3]), cv::Scalar(255, 255, 255), 2);
    }
    showImage(ui->showPic2,drawlines,true);
    return;
//    Mat gray=image.clone();
//    gray.convertTo(gray,CV_32FC1);
//    cv::Mat gradient_x;
//    gradient_x=(cv::Mat_<float>(3,3)<<-1,0,1,
//                                      -2,0,2,
//                                      -1,0,1);
//    cv::Mat gradient_y;
//    gradient_y=(cv::Mat_<float>(3,3)<<-1,-2,-1,
//                                       0,0,0,
//                                       1,2,1);
//    cv::Mat g_x;
//    g_x=cv::Mat::zeros(gray.size(),CV_32FC1);
//    cv::Mat g_y;
//    g_y=cv::Mat::zeros(gray.size(),CV_32FC1);

//    //使用卷积计算x方向 y方向的梯度
//    cv::filter2D(gray,g_x,-1,gradient_x);
//    cv::filter2D(gray,g_y,-1,gradient_y);

//    cv::Mat Mag(gray.size(),CV_32FC1);
//    cv::Mat angle(gray.size(),CV_32FC1);

//    //计算幅度与角度
//    for(int i=0;i<g_x.rows;i++)
//    {
//        for(int j=0;j<g_x.cols;j++)
//        {
//            float gx=g_x.at<float>(i,j);
//            float gy=g_y.at<float>(i,j);
//            Mag.at<float>(i,j)=std::sqrt(std::pow(gx,2)+std::pow(gy,2));
//            angle.at<float>(i,j)=std::atan2(gy,gx);
//        }
//    }
//    cv::Mat dst;
//    dst=cv::Mat::zeros(Mag.size(),CV_8U);


//    //根据范围赋予不同的值
//    for(int i=1;i<Mag.rows-1;i++){
//        for(int j=1;j<Mag.cols-1;j++){
//            float magx=Mag.at<float>(i,j);
//            float anglex=angle.at<float>(i,j);
//            for(int m=i-1;m<=i+1;m++){
//                for(int n=j-1;n<=j+1;n++){
//                    float magy=Mag.at<float>(m,n);
//                    float angley=angle.at<float>(m,n);
//                    if(m!=i&&n!=j)
//                    {
//                        if (std::abs(magx - magy) <= 30 && std::abs(anglex - angley) <= 5) {
//                            int dd = (int) (magx / (1.4));//最大的值为sqrt(2)*255
//                            dst.at<uchar>(m, n) = dd;
//                        }
//                    }
//                }
//            }
//        }
//    }
//    showImage(ui->showPic2,dst,true);
//    return;
}




void Segmentation::on_hough_clicked() //霍夫变换检测直线
{
    if(image.rows==0)
    {
        QMessageBox::warning(this,"提示","没有打开图片");
        ui->detect->setCurrentText("原图");
        return;
    }
    Mat src = image.clone();
    Mat binary;
    Mat canny;
    Mat drawlines = Mat(image.rows,image.cols,CV_8UC3,Scalar::all(255));

    cv::threshold(src, binary, 125, 255, cv::THRESH_BINARY); //二值化
    cv::Canny(binary, canny, 50, 125, 3); //边缘检测
    std::vector<cv::Vec4i> lines; //检测线段
    HoughLinesP(canny,lines,1,CV_PI/180,50,0,50); //使用渐进概率式霍夫变换
     //画线
    for (size_t i = 0; i < lines.size(); i++)
    {
        cv::Vec4i& linex = lines[i];
//        int dx=linex[2]-linex[0];
//        int dy=linex[2]-linex[1];
//        double angle = atan2(double(dy),dx) * 180 /CV_PI;
//        qDebug()<<angle;
        line(drawlines, cv::Point(linex[0], linex[1]), cv::Point(linex[2], linex[3]), cv::Scalar(0, 0, 255), 2);
    }
    showImage(ui->showPic2,drawlines,true);
    return;
}
