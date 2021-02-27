#include "enhance.h"
#include "ui_enhance.h"

Enhance::Enhance(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Enhance)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_QuitOnClose,false); //设置父窗口关闭后同时退出
}

Enhance::~Enhance()
{
    delete ui;
}

void Enhance::showImage(QLabel* ql,Mat image,bool scaled) //在指定组件上展示图片
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


void Enhance::on_openPic_clicked() //打开图片
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
    //processedimage = image.clone();在这里clone会导致闪退
    showImage(ui->showPic1,image,true);
    return;
}

void Enhance::recover()
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

void Enhance::on_smooth_currentTextChanged(const QString &text) //图像平滑
{
    if(image.rows==0)
    {
        QMessageBox::warning(this,"提示","没有打开图片");
        ui->smooth->setCurrentText("原图");
        return;
    }
    recover();
    QString ntext = text.split(" ")[0];//取空格之前的内容
    if(ntext != "原图")
    {

        if(ntext == "均值滤波")
        {
            for (int i = 1; i < processedimage.rows; ++i)
            for (int j = 1; j < processedimage.cols; ++j)
            {
                if ((i - 3) >= 0 && (j - 3) >= 0 && (i + 3)<processedimage.rows && (j + 3)<processedimage.cols)
                {
                    //7*7方块中心点的灰度为全部点灰度值的均值
                    int tot = 0;
                    for(int m=i-3;m<=i+3;m++)
                    {
                        for(int n=j-3;n<=j+3;n++)
                        {
                            tot += image.at<uchar>(m,n);
                        }
                    }
                    processedimage.at<uchar>(i,j) = (uchar)(tot/49);
                }
                else
                {
                    //边缘
                    processedimage.at<uchar>(i, j) = image.at<uchar>(i, j);
                }
            }
        }
        else if(ntext == "中值滤波")
        {
            for (int i = 1; i < processedimage.rows; ++i)
            for (int j = 1; j < processedimage.cols; ++j)
            {
                if ((i - 3) >= 0 && (j - 3) >= 0 && (i + 3)<processedimage.rows && (j + 3)<processedimage.cols)
                {
                    //7*7方块中心点的灰度为全部点灰度值的中值
                    vector<uchar> all;
                    for(int m=i-3;m<=i+3;m++)
                    {
                        for(int n=j-3;n<=j+3;n++)
                        {
                            all.push_back(image.at<uchar>(m,n));
                        }
                    }
                    std::sort(all.begin(),all.end());
                    processedimage.at<uchar>(i,j) = all[24];
                }
                else
                {
                    //边缘
                    processedimage.at<uchar>(i, j) = image.at<uchar>(i, j);
                }
            }
        }
        else if(ntext == "k近邻中值滤波")
        {
            for (int i = 1; i < processedimage.rows; ++i)
            for (int j = 1; j < processedimage.cols; ++j)
            {
                if ((i - 3) >= 0 && (j - 3) >= 0 && (i + 3)<processedimage.rows && (j + 3)<processedimage.cols)
                {
                    //7*7方块中心点的灰度为全部点中k个邻近点的灰度的中值（k=25）
                    vector<uchar> all;
                    for(int m=i-3;m<=i+3;m++)
                    {
                        for(int n=j-3;n<=j+3;n++)
                        {
                            all.push_back(image.at<uchar>(m,n));
                        }
                    }
                    std::sort(all.begin(),all.end());
                    int p = image.at<uchar>(i,j);
                    for(int tt=0;tt<24;tt++) //淘汰掉24个点
                    {
                        if( abs(p-*all.begin()) <= abs(p-*(all.end()-1))) all.erase(all.end()-1);
                        else all.erase(all.begin());
                    }
                    processedimage.at<uchar>(i,j) = all[12];
                }
                else
                {
                    //边缘
                    processedimage.at<uchar>(i, j) = image.at<uchar>(i, j);
                }
            }
        }
    }
    showImage(ui->showPic2,processedimage,true);
}

void Enhance::on_sharpen_currentTextChanged(const QString &text) //图像锐化
{
    if(image.rows==0)
    {
        QMessageBox::warning(this,"提示","没有打开图片");
        ui->sharpen->setCurrentText("原图");
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

void Enhance::on_convolution_clicked() //任意模板卷积
{
    if(image.rows==0)
    {
        QMessageBox::warning(this,"提示","没有打开图片");
        return;
    }
    QString txt = ui->textEdit->toPlainText();
    int row=0;
    for(int i=0;i<txt.length();i++)
    {
        if(txt[i]=='\n')
        {
            row++;
            txt[i]=' ';
        }
    }
    QStringList list = txt.split(" ");
    list.erase(list.end()-1); //删掉末尾的空格
    int col=list.length()/row;
//    qDebug()<<row<<' '<<col<<' '<<list.length();
//    qDebug()<<list;
    if(row*col != list.length())
    {
        QMessageBox::warning(this,"提示","输入不合法");
        return;
    }
    Mat kernel = Mat(row,col,DataType<char>::type);
    for(int i=0;i<row;i++)
    {
        for(int j=0;j<col;j++)
        {
            kernel.at<char>(i,j)=list[i*row+j].toInt();
        }
    }
    filter2D(image,processedimage,image.depth(),kernel);
    showImage(ui->showPic2,processedimage,true);
}

void Enhance::on_homo_clicked() //同态滤波
{
    if(image.rows==0)
    {
        QMessageBox::warning(this,"提示","没有打开图片");
        return;
    }

    Mat srcImg = image.clone();
    Mat dst(srcImg.rows, srcImg.cols, srcImg.type());

    srcImg.convertTo(srcImg, CV_64FC1);
    dst.convertTo(dst, CV_64FC1);
    for (int i = 0; i < srcImg.rows; i++)
    {
        double* srcdata = srcImg.ptr<double>(i);
        double* logdata = srcImg.ptr<double>(i);
        for (int j = 0; j < srcImg.cols; j++)
        {
            logdata[j] = log(srcdata[j]+0.0001);
        }
    }

    //频谱图
    Mat mat_dct = Mat::zeros(srcImg.rows, srcImg.cols, CV_64FC1);
    dct(srcImg, mat_dct);

    Mat H_u_v;
    double gammaH = 1.5;
    double gammaL = 0.5;
    double C = 1;
    double d0 = (srcImg.rows/2)*(srcImg.rows/2) + (srcImg.cols/2)*(srcImg.cols/2);
    double d2 = 0;
    H_u_v = Mat::zeros(srcImg.rows, srcImg.cols, CV_64FC1);

    double totalWeight = 0.0;
    for (int i = 0; i < srcImg.rows; i++)
    {
        double * dataH_u_v = H_u_v.ptr<double>(i);
        for (int j = 0; j < srcImg.cols; j++)
        {
            d2 = pow((i), 2.0) + pow((j), 2.0);
            dataH_u_v[j] = 	(gammaH - gammaL)*(1 - exp(-C*d2/d0)) + gammaL;
            totalWeight += dataH_u_v[j];
        }
    }
    H_u_v.ptr<double>(0)[0] = 1.1;

    mat_dct = mat_dct.mul(H_u_v);
    idct(mat_dct, dst);

    for (int i = 0; i < srcImg.rows; i++)
    {
        double* srcdata = dst.ptr<double>(i);
        double* dstdata = dst.ptr<double>(i);
        for (int j = 0; j < srcImg.cols; j++)
        {
            dstdata[j] = exp(srcdata[j]);
        }
    }

    dst.convertTo(dst, CV_8UC1);
    showImage(ui->showPic2,dst,true);
}


