//
//  openCamera.cpp
//
//  Created by CHAOJIE LYU on 2017/12/23.
//  Copyright © 2017年 CHAOJIE LYU. All rights reserved.
//


#include <opencv.hpp>
#include <iostream>
using namespace std;
using namespace cv;

int DF = 128;

inline float sqr(float a){return a*a;}
inline float dis(Vec3f a, Vec3f b){
    return sqr(a[0]-b[0])+sqr(a[1]-b[1])+sqr(a[2]-b[2]);
    //float aa = sqr(a[0])+sqr(a[1])+sqr(a[2]);
    //float bb = sqr(b[0])+sqr(b[1])+sqr(b[2]);
    //return (a[0]*b[0]+a[1]*b[1]+a[2]*b[2])/sqrt(aa)/sqrt(bb);
}
Vec3f BGR2HSV(Vec3b a){
    float r = a[2];
    float b = a[0];
    float g = a[1];
    
    int Min = min(r,min(g,b));
    int Max = max(r,max(g,b));
    
    Vec3f ret;
    ret[2] = Max;
    ret[1] = (Max==0)?0:1-(float)Min/Max;
    
    if (Min==Max) ret[0] = 0;
    else if (Max==r && g>=b) ret[0] = 60*(g-b)/(Max-Min);
    else if (Max==r && g< b) ret[0] = 60*(g-b)/(Max-Min)+360;
    else if (Max==g) ret[0] = 60*(b-r)/(Max-Min)+120;
    else if (Max==b) ret[0] = 60*(r-g)/(Max-Min)+240;
    
    return ret;
}
struct MarioColor{
    Vec3b color[16];
    void init()
    {
        /*color[0]=Vec3b(255,151,95);
        color[1]=Vec3b(15,79,203);
        color[2]=Vec3b(0,171,0);
        color[3]=Vec3b(179,191,255);
        color[4]=Vec3b(255,255,255);
        color[5]=Vec3b(255,11,63);
        color[6]=Vec3b(0,43,219);
        color[7]=Vec3b(59,155,255);
        //color[8]=Vec3b(0,115,139);
        color[9]=Vec3b(19,211,131);
        color[10]=Vec3b(0,0,0);
        color[11]=Vec3b(139,131,0);
        color[12]=Vec3b(111,119,44);
        color[13]=Vec3b(118,118,118);
        color[14]=Vec3b(241,252,255);
        color[8]=Vec3b(255,63,63);*/
        int tot=0;
        for (int i=0;i<256/DF;i++)for (int j=0;j<256/DF;j++)for (int k=0;k<256/DF;k++)
            color[tot++]=Vec3b(DF*i?0:DF*i-1,DF*j?0:DF*j-1,DF*k?0:DF*k-1);
    }
    Vec3b getNearest(Vec3b a){
        Vec3b ret = color[0];
        for (int i=1;i<15;i++){
            if (dis(BGR2HSV(color[i]),BGR2HSV(a))<dis(BGR2HSV(ret),BGR2HSV(a))) ret=color[i];
        }
        return ret;
    }
};
//Vec3b是按BGR排列
Vec3b nearest[256][256][256];

MarioColor marioColor;
bool vis[720][1280];

bool myVis[720][1200];
int myClick=0;
int flashTime=0;
int myX,myY;

const int way1[8]={-1,0,1,-1,1,-1,0,1};
const int way2[8]={-1,-1,-1,0,0,1,1,1};
void bfs(int x,int y,int Maxx,int Maxy){
    myVis[x][y]=1;
    for (int i=0;i<8;i++){
        int xx=x+way1[i],yy=y+way2[i];
        if (0<=xx && xx<Maxx && 0<=yy && yy<Maxy && vis[xx][yy]==0 && !myVis[xx][yy]) bfs(xx,yy,Maxx,Maxy);
    }
}
Mat pixel(Mat &frame,int width){//width像素正方形的边长
    // cout<<frame.type()<<endl;
    Mat ret = frame.clone();
    //cout<<ret.rows<<" "<<ret.cols<<endl;720*1280
    memset(vis,0,sizeof vis);
    for (int i=0;i<ret.rows;i++){
        for (int j=0;j<ret.cols;j++){
            Vec3b &rgba_tmp = frame.at<Vec3b>(i/width*width,j/width*width);
            
            Vec3b &rgba = ret.at<Vec3b>(i,j);
            //rgba = nearest[rgba_tmp[0]][rgba_tmp[1]][rgba_tmp[2]];
            rgba[0] = rgba_tmp[0]/DF*DF?rgba_tmp[0]/DF*DF-1:0;
            rgba[1] = rgba_tmp[1]/DF*DF?rgba_tmp[1]/DF*DF-1:0;
            rgba[2] = rgba_tmp[2]/DF*DF?rgba_tmp[2]/DF*DF-1:0;
            
            if ((rgba[0]==127 && rgba[1]==127 && rgba[2]==127) || (rgba[0]==0 && rgba[1]==127 && rgba[2]==127)){
                rgba = frame.at<Vec3b>(i,j);
                vis[i/width][j/width]=1;
            }
            //printf("%d %d %d\n",rgba[0],rgba[1],rgba[2]);
        }
    }
    


    for (int i=0;i<ret.rows/width;i++){
        for (int j=0;j<ret.cols/width;j++){
            int now=0;
            for (int k=0;k<8;k++){
                int ii = i+way1[k], jj=j+way2[k];
                if (ii>=0 && ii<ret.rows/width && jj>=0 && jj<ret.cols/width){
                    now+=1-vis[ii][jj];
                }
            }
            if (now<3) vis[i][j]=1;
        }
    }
    
    memset(myVis,0,sizeof myVis);
    flashTime++;
    if (myClick && flashTime< 100){
        bfs(myX/width,myY/width,ret.rows/width,ret.cols/width);
    }else {
        myClick=0;
        flashTime=0;
    }
    printf("%d\n",flashTime);
    for (int i=0;i<ret.rows;i++){
        for (int j=0;j<ret.cols;j++){
            Vec3b &rgba_tmp = frame.at<Vec3b>(i/width*width,j/width*width);
            
            Vec3b &rgba = ret.at<Vec3b>(i,j);
            //rgba = nearest[rgba_tmp[0]][rgba_tmp[1]][rgba_tmp[2]];
            rgba[0] = rgba_tmp[0]/DF*DF?rgba_tmp[0]/DF*DF-1:0;
            rgba[1] = rgba_tmp[1]/DF*DF?rgba_tmp[1]/DF*DF-1:0;
            rgba[2] = rgba_tmp[2]/DF*DF?rgba_tmp[2]/DF*DF-1:0;
            
            if (vis[i/width][j/width] || myVis[i/width][j/width]){
                rgba = frame.at<Vec3b>(i,j);
            }
            //printf("%d %d %d\n",rgba[0],rgba[1],rgba[2]);
        }
    }
    return ret;
}

void Compare(Mat &now, Mat &last){
    for (int i=0;i<now.rows;i++){
        for (int j=0;j<now.cols;j++){
            Vec3b &rgba_tmp = last.at<Vec3b>(i,j);
            Vec3b &rgba = now.at<Vec3b>(i,j);
            //if (dis(rgba_tmp,rgba)<=DF*DF*3/16/2){
            //    rgba=rgba_tmp;
            //}
            for (int k=0;k<3;k++) if (abs(rgba_tmp[k]-rgba[k])<=10){
                rgba[k]=rgba_tmp[k];
            }
        }
    }
}
void on_MouseHandle(int event, int x, int y, int flags, void* param)
{
    Mat& image = *(cv::Mat*) param;
    switch (event) {
        case EVENT_LBUTTONDOWN:
            myClick=1;
            myX=x;
            myY=y;
            break;
            
        default:
            break;
    }
}
int main()
{
    marioColor.init();
    //for (int i=0;i<256;i++)for (int j=0;j<256;j++)for (int k=0;k<256;k++) nearest[i][j][k]=marioColor.getNearest(Vec3b(i,j,k));
    //for (int i=0;i<256;i++) near[i] = (i+DF-1)/DF*DF?(i+DF-1)/DF*DF-1:0,printf("%d:%d\n",i,near[i]);
    //打开摄像头进行预览的程序
    VideoCapture cap(0);//0,1,2代表摄像头，0代表默认摄像头，也可以写路径，自己电脑上保存的视频
    
    Mat frame[2];//创建两帧，前一帧当前帧
    Mat pix;
    int now=0,last=1,flag=0;//flag=0表示当前是第1帧
    while(true)
    {
        cap>>frame[now];//读取一帧图像保存在frame里
        
        if(!flag) flag=1; else Compare(frame[now],frame[last]);
        
        blur(frame[now],pix,cv::Size(1,1),Point(-1,-1));
        //打马赛克
        pix = pixel(pix,20);
        
        setMouseCallback("result",on_MouseHandle,(void*)&pix);
        //for (int i=0;i<3;i++) ChangeColor(pix);
        //新建一个没有图像的窗口
        namedWindow("result",0);//0 窗口可拖动调大小 1不可拖动
        imshow("result",pix);
        
        waitKey(1);//waitKey()函数的功能是不断刷新图像，频率时间为delay，单位为ms。
        
        now^=1;last^=1;
    }
}

