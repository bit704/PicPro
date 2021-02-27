#include "convert.h"
#include "ui_convert.h"

Convert::Convert(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Convert)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_QuitOnClose,false); //设置父窗口关闭后同时退出
}

Convert::~Convert()
{
    delete ui;
}

void Convert::showImage(QLabel* ql,Mat image,bool scaled) //在指定组件上展示图片
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


void Convert::on_openPic_clicked() //打开图片
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

void Convert::on_spectrum_clicked() //频谱图显示及逆变换
{
    if(image.cols==0)
    {
        QMessageBox::warning(this,"提示","没有打开图片");
        return;
    }
    Mat simage = image.clone();
    int w=getOptimalDFTSize(simage.cols);
    int h=getOptimalDFTSize(simage.rows);//获取最佳尺寸，快速傅立叶变换要求尺寸为2的n次方
    Mat padded;     //将输入图像延扩到最佳的尺寸  在边缘添加0
    copyMakeBorder(simage,padded,0,h-simage.rows,0,w-simage.cols,BORDER_CONSTANT,Scalar::all(0));//填充图像保存到padded中
    Mat plane[]={Mat_<float>(padded),Mat::zeros(padded.size(),CV_32F)};//创建通道
    Mat complexIm;
    merge(plane,2,complexIm);//为延扩后的图像增添一个初始化为0的通道
    dft(complexIm,complexIm);//进行傅立叶变换，结果保存在自身
    split(complexIm,plane);//分离通道
    magnitude(plane[0],plane[1],plane[0]);//获取幅度图像，0通道为实数通道，1为虚数，因为二维傅立叶变换结果是复数
    plane[0] = plane[0](Rect(0, 0, plane[0].cols & -2, plane[0].rows & -2));
    int cx=padded.cols/2;int cy=padded.rows/2;//一下的操作是移动图像，左上与右下交换位置，右上与左下交换位置
    Mat temp;
    Mat part1(plane[0],Rect(0,0,cx,cy));
    Mat part2(plane[0],Rect(cx,0,cx,cy));
    Mat part3(plane[0],Rect(0,cy,cx,cy));
    Mat part4(plane[0],Rect(cx,cy,cx,cy));

    part1.copyTo(temp);
    part4.copyTo(part1);
    temp.copyTo(part4);
    part2.copyTo(temp);
    part3.copyTo(part2);
    temp.copyTo(part3);

    plane[0]+=Scalar::all(1);//傅立叶变换后的图片不好分析，进行对数处理，结果比较好看
    log(plane[0],plane[0]);
    normalize(plane[0],plane[0],0,1,NORM_MINMAX);

    Mat _complexim;
    complexIm.copyTo(_complexim);//把变换结果复制一份，进行逆变换，也就是恢复原图
    Mat iDft[]={Mat::zeros(plane[0].size(),CV_32F),Mat::zeros(plane[0].size(),CV_32F)};//创建两个通道，类型为float，大小为填充后的尺寸
    idft(_complexim,_complexim);//傅立叶逆变换
    split(_complexim,iDft);//结果是复数
    magnitude(iDft[0],iDft[1],iDft[0]);//分离通道，主要获取0通道
    normalize(iDft[0], iDft[0], 0, 1, NORM_MINMAX);

    namedWindow("逆变换",WINDOW_AUTOSIZE);
    imshow("逆变换",iDft[0]);//显示逆变换

    namedWindow("频谱图",WINDOW_AUTOSIZE);
    imshow("频谱图",plane[0]);

    return;
}


Mat Convert::gaussian_lowpass_filter(Mat &src, float sigma) //高斯低通滤波器
{
    //延扩到最佳的尺寸
    int w=getOptimalDFTSize(src.cols);
    int h=getOptimalDFTSize(src.rows);
    Mat padded;
    copyMakeBorder(src,padded,0,h-src.rows,0,w-src.cols,BORDER_CONSTANT,Scalar::all(0));
    padded.convertTo(padded,CV_32FC1);

    Mat gaussianBlur(padded.size(),CV_32FC1); //，CV_32FC1
    float d0=2*sigma*sigma;//高斯函数参数，越小，频率高斯滤波器越窄，滤除高频成分越多，图像就越平滑
    for(int i=0;i<padded.rows ; i++ ){
        for(int j=0; j<padded.cols ; j++ ){
            float d=pow(float(i-padded.rows/2),2)+pow(float(j-padded.cols/2),2);//分子,计算pow必须为float型
            gaussianBlur.at<float>(i,j)=expf(-d/d0);//expf为以e为底求幂（必须为float型）
        }
    }

    Mat result = frequency_filter(padded,gaussianBlur);
    return result;
}

void Convert::on_lowpass_clicked() //高斯低通滤波
{
    if(image.cols==0)
    {
        QMessageBox::warning(this,"提示","没有打开图片");
        return;
    }
    Mat input = image.clone();
    cv::Mat gaussion_low = gaussian_lowpass_filter(input, 30);
    gaussion_low = gaussion_low(cv::Rect(0,0, input.cols, input.rows));
    namedWindow("高斯低通滤波",WINDOW_AUTOSIZE);
    imshow("高斯低通滤波", gaussion_low);
}


Mat Convert::gaussian_highpass_filter(Mat &src, float sigma)  //高斯高通滤波器
{
    //延扩到最佳的尺寸
    int w=getOptimalDFTSize(src.cols);
    int h=getOptimalDFTSize(src.rows);
    Mat padded;
    copyMakeBorder(src,padded,0,h-src.rows,0,w-src.cols,BORDER_CONSTANT,Scalar::all(0));
    padded.convertTo(padded,CV_32FC1);

    Mat gaussianBlur(padded.size(),CV_32FC1); //，CV_32FC1
    float d0=2*sigma*sigma;
    for(int i=0;i<padded.rows ; i++ ){
        for(int j=0; j<padded.cols ; j++ ){
            float d=pow(float(i-padded.rows/2),2)+pow(float(j-padded.cols/2),2);//分子,计算pow必须为float型
            gaussianBlur.at<float>(i,j)=1-expf(-d/d0);
        }
    }

    Mat result = frequency_filter(padded,gaussianBlur);
    return result;
}

void Convert::on_highpass_clicked() //高斯高通滤波
{
    if(image.cols==0)
    {
        QMessageBox::warning(this,"提示","没有打开图片");
        return;
    }
    Mat input = image.clone();
    cv::Mat gaussion_high = gaussian_highpass_filter(input, 80);
    gaussion_high = gaussion_high(cv::Rect(0,0, input.cols, input.rows));
    namedWindow("高斯高通滤波",WINDOW_AUTOSIZE);
    imshow("高斯高通滤波", gaussion_high);
}

Mat Convert::frequency_filter(Mat &padded,Mat &blur) //频率域滤波
{
    //DFT
    Mat plane[]={padded, Mat::zeros(padded.size() , CV_32FC1)}; //创建通道，存储dft后的实部与虚部（CV_32F，必须为单通道数）
    Mat complexIm;
    merge(plane,2,complexIm);//合并通道 （把两个矩阵合并为一个2通道的Mat类容器）
    dft(complexIm,complexIm);//进行傅立叶变换，结果保存在自身

    //中心化
    split(complexIm,plane);//分离通道（数组分离）
    //plane[0] = plane[0](Rect(0, 0, plane[0].cols & -2, plane[0].rows & -2));//这里为什么&上-2具体查看opencv文档
    //其实是为了把行和列变成偶数 -2的二进制是11111111.......10 最后一位是0
    int cx=plane[0].cols/2;int cy=plane[0].rows/2;//以下的操作是移动图像  (零频移到中心)
    Mat part1_r(plane[0],Rect(0,0,cx,cy));  //元素坐标表示为(cx,cy)
    Mat part2_r(plane[0],Rect(cx,0,cx,cy));
    Mat part3_r(plane[0],Rect(0,cy,cx,cy));
    Mat part4_r(plane[0],Rect(cx,cy,cx,cy));

    Mat temp;
    part1_r.copyTo(temp);  //左上与右下交换位置(实部)
    part4_r.copyTo(part1_r);
    temp.copyTo(part4_r);

    part2_r.copyTo(temp);  //右上与左下交换位置(实部)
    part3_r.copyTo(part2_r);
    temp.copyTo(part3_r);

    Mat part1_i(plane[1],Rect(0,0,cx,cy));  //元素坐标(cx,cy)
    Mat part2_i(plane[1],Rect(cx,0,cx,cy));
    Mat part3_i(plane[1],Rect(0,cy,cx,cy));
    Mat part4_i(plane[1],Rect(cx,cy,cx,cy));

    part1_i.copyTo(temp);  //左上与右下交换位置(虚部)
    part4_i.copyTo(part1_i);
    temp.copyTo(part4_i);

    part2_i.copyTo(temp);  //右上与左下交换位置(虚部)
    part3_i.copyTo(part2_i);
    temp.copyTo(part3_i);

    //滤波器函数与DFT结果的乘积
    Mat blur_r,blur_i,BLUR;
    multiply(plane[0], blur, blur_r); //滤波（实部与滤波器模板对应元素相乘）
    multiply(plane[1], blur,blur_i);//滤波（虚部与滤波器模板对应元素相乘）
    Mat plane1[]={blur_r, blur_i};
    merge(plane1,2,BLUR);//实部与虚部合并

    //得到原图频谱图
    magnitude(plane[0],plane[1],plane[0]);//获取幅度图像，0通道为实部通道，1为虚部，因为二维傅立叶变换结果是复数
    plane[0]+=Scalar::all(1);  //傅立叶变换后的图片不好分析，进行对数处理，结果比较好看
    log(plane[0],plane[0]);    // float型的灰度空间为[0，1])
    normalize(plane[0],plane[0],1,0,NORM_MINMAX);  //归一化便于显示

    idft( BLUR, BLUR);    //idft结果也为复数
    split(BLUR,plane);//分离通道，主要获取通道
    magnitude(plane[0],plane[1],plane[0]);  //求幅值(模)
    normalize(plane[0],plane[0],1,0,NORM_MINMAX);  //归一化便于显示
    return plane[0];//返回参数
}

void Convert::on_bandpass_clicked() //带通滤波
{
    if(image.cols==0)
    {
        QMessageBox::warning(this,"提示","没有打开图片");
        return;
    }
    Mat band = image.clone();
    band = gaussian_highpass_filter(band, 90);
    band = gaussian_lowpass_filter(band, 20);
    band = band(cv::Rect(0,0, image.cols, image.rows));
    namedWindow("带通滤波",WINDOW_AUTOSIZE);
    imshow("带通滤波", band);
}

void Convert::on_dct_clicked() //离散余弦变换
{
    if(image.cols==0)
    {
        QMessageBox::warning(this,"提示","没有打开图片");
        return;
    }
    Mat src = image.clone();
    cv::resize(src, src, Size(image.rows, image.cols));
    src.convertTo(src, CV_32F, 1.0/255);
    cv::dct(src,src);
    Mat isrc = src.clone();
    cv::idct(src,isrc);

    namedWindow("逆变换",WINDOW_AUTOSIZE);
    imshow("逆变换", isrc);
    namedWindow("DCT系数",WINDOW_AUTOSIZE);
    imshow("DCT系数", src);
    return;
}

void Convert::on_wt_clicked() //小波变换
{
    if(image.cols==0)
    {
        QMessageBox::warning(this,"提示","没有打开图片");
        return;
    }
    Mat img = image.clone();
    int Height = img.cols;
    int Width = img.rows;
    int depth = 2;    //分解层数
    int depthcount = 1;
    Mat tmp = Mat::ones(Width, Height, CV_32FC1);
    Mat wavelet = Mat::ones(Width, Height, CV_32FC1);
    Mat imgtmp = img.clone();
    imgtmp.convertTo(imgtmp, CV_32FC1);
    while (depthcount<=depth){
        Width = img.rows / depthcount;
        Height = img.cols / depthcount;

        for (int i = 0; i < Width; i++){
            for (int j = 0; j < Height / 2; j++){
                tmp.at<float>(i, j) = (imgtmp.at<float>(i, 2 * j) + imgtmp.at<float>(i, 2 * j + 1)) / 2;
                tmp.at<float>(i, j + Height / 2) = (imgtmp.at<float>(i, 2 * j) - imgtmp.at<float>(i, 2 * j + 1)) / 2;
            }
        }
        for (int i = 0; i < Width / 2; i++){
            for (int j = 0; j < Height; j++){
                wavelet.at<float>(i, j) = (tmp.at<float>(2 * i, j) + tmp.at<float>(2 * i + 1, j)) / 2;
                wavelet.at<float>(i + Width / 2, j) = (tmp.at<float>(2 * i, j) - tmp.at<float>(2 * i + 1, j)) / 2;
            }
        }
        imgtmp = wavelet;
        depthcount++;
    }
    wavelet.convertTo(wavelet, CV_8UC1);
    wavelet += 90; //防止图像过暗看不清
    namedWindow("小波变换",WINDOW_AUTOSIZE);
    imshow("小波变换", wavelet);
    waitKey(0);
    return;

}
