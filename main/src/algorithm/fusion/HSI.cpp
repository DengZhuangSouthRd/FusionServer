#include "HSI.h"

//将RGB图像转换到HSI空间 MS_RGB_Mat各行依次为R、G、B
void HSI::RGB2HSI(float *MS_RGB_Mat,int height,int width,int bandcount){

    //RGB2HSI	将图像从RGB空间转换到HSI空间
    //MS_RGB_Mat各行依次为R、G、B
    //R、G、B各行依次变换为H、S、I
    //修改内容：考虑节约内存，不返回值，而是在原数据基础上做变换
    //日期：2015.11.12
    //作者：YS

    if (bandcount != 3)
    {
        cerr<<"RGB to HSI Error."<<endl;
        cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
        throw "rgb2his error";
    }
    int i;
    int pixelcount = height*width; //每个波段的像素数

    float MS_r, MS_g, MS_b;
    float min_rgb;
    float fHue, fSaturation, fIntensity;
    for (i = 0; i < pixelcount; i++)
    {
        MS_r = DATA2D(MS_RGB_Mat, 0, i, pixelcount);
        MS_g = DATA2D(MS_RGB_Mat, 1, i, pixelcount);
        MS_b = DATA2D(MS_RGB_Mat, 2, i, pixelcount);;

        // Intensity分量[0, 1]
        fIntensity = ((MS_b + MS_g + MS_r) / 3);

        // 得到RGB分量中的最小值
        float fTemp = MS_r < MS_g ? MS_r : MS_g;
        min_rgb = fTemp < MS_b ? fTemp : MS_b;
        // Saturation分量[0, 1]
        fSaturation = 1 - (float)(3 * min_rgb) / (MS_r + MS_g + MS_b);

        // 计算theta角
        float numerator = (MS_r - MS_g + MS_r - MS_b) / 2;
        float denominator = sqrt(pow((MS_r - MS_g), 2) + (MS_r - MS_b)*(MS_g - MS_b));

        // 计算Hue分量
        if (denominator != 0)
        {
            float theta = acos(numerator / denominator) * 180 / 3.14;

            if (MS_b <= MS_g)
            {
                fHue = theta;
            }
            else
            {
                fHue = 360 - theta;
            }
        }
        else
        {
            fHue = 0;
        }
        // 赋值
        DATA2D(MS_RGB_Mat, 0, i, pixelcount) = fHue;
        DATA2D(MS_RGB_Mat, 1, i, pixelcount) = fSaturation;
        DATA2D(MS_RGB_Mat, 2, i, pixelcount) = fIntensity;
    }


}

//将HSI图像转换到RGB空间 MS_HSI_Mat各行依次为H、S、I
void HSI::HSI2RGB(float *MS_HSI_Mat, int height, int width, int bandcount){

    //HSI2RGB	将图像从HSI空间转换到RGB空间
    //MS_HSI_Mat各行依次为H、S、I
    //H、S、I各行依次变换为R、G、B
    //修改内容：考虑节约内存，不返回值，而是在原数据基础上做变换
    //日期：2015.11.12
    //作者：YS

    if (bandcount != 3)
    {
        cerr<<"HSI to RGB Error."<<endl;
        cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
        throw "his2rgb error";
    }

    int i;

    int pixelcount = height*width;//每个波段的像素数


    float fB, fG, fR;
    for (i = 0; i < pixelcount; i++)
    {
        // 该点的色度H
        float fH = DATA2D(MS_HSI_Mat, 0, i, pixelcount);
        // 该点的色饱和度S
        float fS = DATA2D(MS_HSI_Mat, 1, i, pixelcount);
        // 该点的亮度I
        float fI = DATA2D(MS_HSI_Mat, 2, i, pixelcount);

        float fTempB, fTempG, fTempR;
        // RG扇区
        if (fH < 120 && fH >= 0)
        {
            // 将H转为弧度表示
            fH = fH * 3.1415926 / 180;
            fTempB = fI * (1 - fS);
            fTempR = fI * (1 + (fS * cos(fH)) / cos(3.1415926 / 3 - fH));
            fTempG = (3 * fI - (fTempR + fTempB));
        }
        // GB扇区
        else if (fH < 240 && fH >= 120)
        {
            fH -= 120;

            // 将H转为弧度表示
            fH = fH * 3.1415926 / 180;

            fTempR = fI * (1 - fS);
            fTempG = fI * (1 + fS * cos(fH) / cos(3.1415926 / 3 - fH));
            fTempB = (3 * fI - (fTempR + fTempG));
        }
        // BR扇区
        else
        {
            fH -= 240;

            // 将H转为弧度表示
            fH = fH * 3.1415926 / 180;

            fTempG = fI * (1 - fS);
            fTempB = fI * (1 + (fS * cos(fH)) / cos(3.1415926 / 3 - fH));
            fTempR = (3 * fI - (fTempG + fTempB));
        }

        fB = fTempB ;
        fG = fTempG ;
        fR = fTempR ;

        // 赋值
        DATA2D(MS_HSI_Mat, 0, i, pixelcount) = fR;
        DATA2D(MS_HSI_Mat, 1, i, pixelcount) = fG;
        DATA2D(MS_HSI_Mat, 2, i, pixelcount) = fB;
    }

}

