#include "bmp2txt.h"
#include "ui_bmp2txt.h"

Bmp2txt::Bmp2txt(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Bmp2txt)
{
    ui->setupUi(this);
}

Bmp2txt::Bmp2txt(Mat yimage,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Bmp2txt)
{

    ui->setupUi(this);
    setAttribute(Qt::WA_QuitOnClose,false); //设置父窗口关闭后同时退出
    //txtimage=Mat(yimage.rows,yimage.cols,DataType<uchar>::type)
    char char_list[68] = {'$','@','B','%','8','&','W','M','#','*','o','a','h','k','b','d','p','q','w','m','Z',
                          'O','0','Q','L','C','J','U','Y','X','z','c','v','u','n','x','r','j','f','t','/','|',
                          '(',')','1','{','}','[',']','?','-','_','+','~','<','>','i','!','l','I',';',':',',',
                          '"','^','`','.',' '};
    QString txt="";
    for(int i=0;i<yimage.rows;i++)
    {
        for(int j=0;j<yimage.cols;j++)
        {
            uchar g;
            if(yimage.channels()==3) g=yimage.at<Vec3b>(i,j)[0];
            else if(yimage.channels()==1) g=yimage.at<uchar>(i,j);
            if(g>=240) txt.append(' ');
            else txt.append(QChar(char_list[int(g/255.0*67)])); //将每个像素点转化为对应字符
            txt.append(' ');
        }
        txt.append('\n');
    }
    ui->textBrowser->setText(txt);
}

Bmp2txt::~Bmp2txt()
{
    delete ui;
}

