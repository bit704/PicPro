#include "pedestrian.h"
#include "ui_pedestrian.h"

Pedestrian::Pedestrian(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Pedestrian)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_QuitOnClose,false); //设置父窗口关闭后同时退出

    ui->textBrowser->setText("视频信息\n视频名称:\n帧率:\n帧数:\n时长:\n\n行人信息\n行人当前速度:\n行人全程平均速度:\n方向:");
}

Pedestrian::~Pedestrian()
{
    delete ui;
}

void Pedestrian::on_loadvideo_clicked()
{
    QString FileName;
    FileName = QFileDialog::getOpenFileName(this,"选择视频",".","*.mp4");
    if(FileName.isEmpty())
    {
        return;
    }
    string video = FileName.toStdString();
    VideoCapture cap; //视频
    Mat frame; //一帧图像
    double fps,fnum,duration; //帧率、帧数、视频时长(s)
    vector<Rect> regions; //存取检测到的目标位置
    HOGDescriptor hog; // HOG对象,采用默认参数
    hog.setSVMDetector(cv::HOGDescriptor::getDefaultPeopleDetector());   // 采用已经训练好的行人检测SVM分类器
    cap.open(video);
    if (!cap.isOpened())
    {
        qDebug()<<"没有打开"<<endl;
        return;
    }
    else
    {
        fps=cap.get(5);
        fnum=cap.get(7);
        duration=fnum/fps;
    }
    QStringList stringlist = FileName.split("/");
    QString videoname = stringlist[stringlist.size()-1];
    QString videoInformation;
    QString pedestrianInformation;
    videoInformation.sprintf("视频信息\n视频名称:%s\n帧率:%.2f帧/秒\n帧数:%.0f帧\n时长:%.2f秒\n\n",
                             videoname.toStdString().c_str(),fps,fnum,duration); //视频信息
    double time = 0;
    double waittime = 1/fps; //两帧之间的等待时间
    vector<Point> centers;
    double ts=0; //平均速度
    double direction=0;
    while (1)
    {
        time += waittime;
        if (time>=duration) //视频结束
        {
            break;
        }
        cap >> frame;
        if (frame.empty())  break;
        hog.detectMultiScale(frame, regions, 0, cv::Size(8, 8), cv::Size(2,2), 1.05, 2);
        //检测行人区域,输入，目标位置，步长，padding，scale,.finalThreshold
        for (size_t i = 0; i < regions.size(); i++) // 显示,框出行人
        {
            cv::rectangle(frame, regions[i], cv::Scalar(0, 0, 255), 5,8);
        }

        Point p;
        p.x=regions[0].x+cvRound(regions[0].width/2.0);
        p.y=regions[0].y+cvRound(regions[0].height/2.0);
        centers.push_back(p);
        double ns;//当前速度
        if(centers.size()<3) ns=0;
        else
        {
            int l=centers.size();
            double d=sqrt(pow(centers[l-1].x-centers[l-2].x,2)+pow(centers[l-1].y-centers[l-2].y,2))
                    +sqrt(pow(centers[l-2].x-centers[l-3].x,2)+pow(centers[l-2].y-centers[l-3].y,2));

            d=d/530*2;
            ns=d/waittime/2;
            direction = atan(centers[l-1].y-centers[l-3].y/centers[l-1].x-centers[l-3].x)/3.14*180;
        }
        ts+=ns;
        pedestrianInformation.sprintf("行人信息\n行人当前速度:%.2f米/秒\n行人全程平均速度:%s\n方向:%.2f度",
                                 ns,"视频放完显示",direction); //行人信息
        ui->textBrowser->setText(videoInformation+pedestrianInformation);
        imshow("Video", frame);
        waitKey(waittime*1000); //延时,单位为ms
    }
    ts = ts/fnum;
    pedestrianInformation.sprintf("行人信息\n行人当前速度:%s\n行人全程平均速度:%.2f米/秒\n方向:%.2f度",
                             "行人运动已结束",ts,direction); //行人信息
    ui->textBrowser->setText(videoInformation+pedestrianInformation);
    cv::waitKey(10);
    return;
}
