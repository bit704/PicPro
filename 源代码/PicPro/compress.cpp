#include "compress.h"
#include "ui_compress.h"

Compress::Compress(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Compress)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_QuitOnClose,false); //设置父窗口关闭后同时退出

    ui->textEdit->setWordWrapMode(QTextOption::NoWrap); //滚动条显示
}

Compress::~Compress()
{
    delete ui;
}


void Compress::on_compress_clicked()
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
    int gray[256]; //灰度对应像素点数目
    for(int i=0;i<256;i++) gray[i]=0;
    for(int i=0;i<image.rows;i++)
    {
        for(int j=0;j<image.cols;j++)
        {
            int g = image.at<uchar>(i,j);
            gray[g]++;
        }
    }
    int nodenum=0; //huffman叶子结数
    for(int i=0;i<256;i++) if(gray[i]>0) nodenum++;

    typedef struct   //节点数据结构
    {
        int g; //代表灰度
        float weight;
        int lchild,rchild,parent;
    }huffmantree;
    huffmantree tree[2*nodenum-1];
    for(int i=0;i<2*nodenum-1;i++) //初始化结点
    {

        tree[i].g=-1;
        tree[i].parent=-1;
        tree[i].lchild=-1;
        tree[i].rchild=-1;
        tree[i].weight=0.0;
    }
    int temp=0;
    for(int i=0;i<256;i++)
    if(gray[i]>0) //读入nodenum个结点
    {
        tree[temp].g=i;
        tree[temp].weight=(float)gray[i]/(image.cols*image.rows); //权值为该灰度点数目占整个图点数目的比例
        temp++;
    }
    int p1,p2;//权值最小和次小的节点的编号
    float minest,min; //最小和次小的权值
    for(int i=nodenum;i<2*nodenum-1;i++) //节点合并，生成树
    {
        p1=p2=0;
        minest=min=1;   //初始化为权重最大值
        for(int j=0;j<i;j++)    //选出两个权值最小的根结点
        {
            if(tree[j].parent==-1)
            {
                if(tree[j].weight<minest)
                {
                    min=minest;
                    minest=tree[j].weight;
                    p2=p1;
                    p1=j;  //改变最小权、次小权节点
                }
                else if(tree[j].weight<min)
                {
                    min=tree[j].weight;
                    p2=j;  //改变次小权节点
                }
            }
        }
        tree[p1].parent=i;
        tree[p2].parent=i;
        tree[i].lchild=p1;  //最小权根结点是新结点的左孩子
        tree[i].rchild=p2;  //次小权根结点是新结点的右孩子
        tree[i].weight=tree[p1].weight+tree[p2].weight;
    }


    typedef struct
    {
        string bits;   //二进制位串
        int g;        //对应灰度
    }codetype;

    codetype code[nodenum];
    for(int i=0;i<nodenum;i++)
    {
        int p; //父节点编号
        int c=i;//当前节点编号
        code[i].g=tree[i].g;
        p=tree[i].parent;   //tree[p]是tree[i]的父节点
        while(p!=-1)
        {
            if(tree[p].lchild==c)
            code[i].bits = '0' + code[i].bits;   //左子树记'0'
            else
            code[i].bits= '1' + code[i].bits;   //右子树记'1'
            c=p;
            p=tree[p].parent;
        }
    }

    int number[256];
    for(int i=0;i<nodenum;i++)
    {
        string tmp =  code[i].bits;
        int num = 0;
        int base = std::pow(2,tmp.size()-1);
        for(size_t i=0; i<tmp.size(); i++)
        {
            if(tmp[i]=='1') num+=base;
            base /= 2;
        }
        number[code[i].g] = num;
    }

    picFileName = QFileDialog::getSaveFileName(this,"保存压缩文件",".","文件 (*)");
    if(picFileName.isEmpty())
    {
        return;
    }
    ofstream outFile(string((const char *)picFileName.toLocal8Bit()),ios::out | ios::binary);
    for(int i=0;i<image.rows;i++)
    {
        for(int j=0;j<image.cols;j++)
        {
            int tmp = number[image.at<uchar>(i,j)];
            outFile.write((char*)&tmp, sizeof(tmp));
        }
    }

    return;
}


void Compress::on_tree_clicked() //图形显示
{
    QString picFileName;
    picFileName = QFileDialog::getOpenFileName(this,"选择图像",".","Images (*.png *.bmp *.jpg *.tif *.GIF )");
    if(picFileName.isEmpty())
    {
        return;
    }
    image=imread(std::string((const char *)picFileName.toLocal8Bit()),IMREAD_GRAYSCALE);

    int gray[256]; //灰度对应像素点数目
    for(int i=0;i<256;i++) gray[i]=0;
    for(int i=0;i<image.rows;i++)
    {
        for(int j=0;j<image.cols;j++)
        {
            int g = image.at<uchar>(i,j);
            gray[g]++;
        }
    }
    int nodenum=0; //huffman叶子结数
    for(int i=0;i<256;i++) if(gray[i]>0) nodenum++;

    typedef struct   //节点数据结构
    {
        int g; //代表灰度
        float weight;
        int lchild,rchild,parent;
    }huffmantree;
    huffmantree tree[2*nodenum-1];
    for(int i=0;i<2*nodenum-1;i++) //初始化结点
    {

        tree[i].g=-1;
        tree[i].parent=-1;
        tree[i].lchild=-1;
        tree[i].rchild=-1;
        tree[i].weight=0.0;
    }
    int temp=0;
    for(int i=0;i<256;i++)
    if(gray[i]>0) //读入nodenum个结点
    {
        tree[temp].g=i;
        tree[temp].weight=(float)gray[i]/(image.cols*image.rows); //权值为该灰度点数目占整个图点数目的比例
        temp++;
    }
    int p1,p2;//权值最小和次小的节点的编号
    float minest,min; //最小和次小的权值
    for(int i=nodenum;i<2*nodenum-1;i++) //节点合并，生成树
    {
        p1=p2=0;
        minest=min=1;   //初始化为权重最大值
        for(int j=0;j<i;j++)    //选出两个权值最小的根结点
        {
            if(tree[j].parent==-1)
            {
                if(tree[j].weight<minest)
                {
                    min=minest;
                    minest=tree[j].weight;
                    p2=p1;
                    p1=j;  //改变最小权、次小权节点
                }
                else if(tree[j].weight<min)
                {
                    min=tree[j].weight;
                    p2=j;  //改变次小权节点
                }
            }
        }
        tree[p1].parent=i;
        tree[p2].parent=i;
        tree[i].lchild=p1;  //最小权根结点是新结点的左孩子
        tree[i].rchild=p2;  //次小权根结点是新结点的右孩子
        tree[i].weight=tree[p1].weight+tree[p2].weight;
    }

//    for(int i=0;i<nodenum;i++)
//    {
//        qDebug()<<i<<' '<<tree[i].weight;
//    }
//    qDebug()<<tree[nodenum*2-2].weight;

    typedef struct
    {
        string bits;   //二进制位串
        int g;        //对应灰度
    }codetype;


    codetype code[nodenum];
    for(int i=0;i<nodenum;i++)
    {
        int p; //父节点编号
        int c=i;//当前节点编号
        code[i].g=tree[i].g;
        p=tree[i].parent;   //tree[p]是tree[i]的父节点
        while(p!=-1)
        {
            if(tree[p].lchild==c)
            code[i].bits = '0' + code[i].bits;   //左子树记'0'
            else
            code[i].bits= '1' + code[i].bits;   //右子树记'1'
            c=p;
            p=tree[p].parent;
        }
    }

//    for(int i=0;i<nodenum;i++)
//    {
//        cout<<i<<' '<<code[i].bits<<endl;
//    }

    int maxdepth=1;
    float totl = 0;
    for(int i=0;i<nodenum;i++)
    {
        totl += code[i].bits.size();
        if(code[i].bits.size()>maxdepth) maxdepth=code[i].bits.size();
    }
    maxdepth += 1;
    totl /= nodenum;

    typedef struct node{
        int g;
        int gap;      //绘图左间距
        int layer;    //所在层数
    }node;
    vector<node> nodelist;
    int mingap = 0;  //最大左间距,绝对值最大的负数
    for(int i=0;i<2*nodenum-1;i++) //统计各个节点信息
    {
        node tmp;
        tmp.g=tree[i].g;
        tmp.layer=0;
        tmp.gap=0;
        int p= tree[i].parent; //父节点编号
        int c=i;//当前节点编号
        while(p!=-1)
        {
            tmp.layer += 1; //更新层数
            if(tree[p].lchild==c)    //更新绘图左间距
            tmp.gap -= 1;
            else
            tmp.gap += 1;
            c=p;
            p=tree[p].parent;
        }
        nodelist.push_back(tmp);
        mingap = std::min(mingap,tmp.gap);
    }

    std::sort(nodelist.begin(),nodelist.end(),
              [](node a,node b)->bool{
                    if(a.layer==b.layer) return a.gap<b.gap;
                    else return a.layer<b.layer;
               });

    string graph = ""; //绘图
    int space = - mingap; //左边基础空白个数
    int exlayer = -1;  //上个结点层数
    for(int i=0;i<2*nodenum-1;i++) //以文本的形式绘图
    {
        node tmp = nodelist[i];
        if(tmp.layer != exlayer) //开始了新的一行
        {
            exlayer = tmp.layer;
            int nowspace = space + tmp.gap; //调整空白
            graph += "\n";
            for(int i=0;i<nowspace;i++) //每一层左边空白
                graph+="      ";//六个空格
        }
        if(tmp.g==-1) graph += "  ##  ";
        else
        {
            graph += std::to_string(tmp.g);
            for(int i=std::to_string(tmp.g).size();i<6;i++) graph += " "; //每个节点固定长度
        }
        graph+="      "; //节点之前间距
    }

    QString information;
    information.sprintf("Huffman树信息\n\n叶节点数:%d\n树深度:%d\n平均编码长度:%.2f\n\n",nodenum,maxdepth,totl);
    information += "Huffman树绘制如下\n(##为非叶节点；数字为叶子节点，值代表节点对应灰度)\n";
    ui->textEdit->setText(information);
    ui->textEdit->setFontPointSize(6);
    ui->textEdit->append(QString::fromStdString(graph));
    ui->textEdit->setFontPointSize(9);

}
