/********************************************************************
    created:	2015/12/17
    created:	17:12:2015   15:16
    filename: 	D:\code\vs2010\C\Fusion\HSI.h
    file path:	D:\code\vs2010\C\Fusion
    file base:	HSI
    file ext:	h
    author:		YS

    purpose:	HSI类声明
                HSI变换与反变换
*********************************************************************/
#ifndef _HSI_H_
#define _HSI_H_


#include <math.h>
#include <iostream>
using namespace std;

#ifndef DATA2D
#define DATA2D(Matrix,m,n,SizeCol) Matrix[(m)*(SizeCol)+(n)]
#endif

class HSI{
public:
    //将RGB图像转换到HSI空间 MS_RGB_Mat各行依次为R、G、B
    void RGB2HSI(float *MS_RGB_Mat,int height,int width,int bandcount);

    //将HSI图像转换到RGB空间 MS_HSI_Mat各行依次为H、S、I
    void HSI2RGB(float *MS_HSI_Mat, int height, int width, int bandcount);
};


#endif
