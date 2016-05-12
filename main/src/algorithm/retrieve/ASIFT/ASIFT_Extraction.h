#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <vector>
using namespace std;

//#define  _OPENMP  //定义OpenMP

#ifdef _OPENMP
#include <omp.h>
#endif

#include "GdalIO.h"
#include "demo_lib_sift.h"

#include "library.h"
#include "frot.h"
#include "fproj.h"
#include "compute_asift_keypoints.h"
#include "compute_asift_matches.h"

# define IM_X 160
# define IM_Y 120


#include <string>
#include <fstream>
#include <iostream>

using namespace std;

void ASIFT_Ext_Features_Gdal(const string Output_FileName,string Input_FilePath) {
    /*
    *Input_FilePath     待提取特征的图片路径
    *提取ASIFT特征 并保存到Output_FileName文件中
    *每行记录一幅图片一个特征点的特征
    *作者：YS
    */

    int i,j;
    float *GrayImg=NULL;  //存储灰度图像像素值
    float *Img = NULL;    //存储图像像素值

    int height ;		  //存储图像高度
    int width ;			  //存储图像宽度
    int bandcount;        //存储图像波段数
    float wS = IM_X;
    float hS = IM_Y;
    int  flag_resize=0;   //是否resize


    //将特征保存文件
    ofstream fout(Output_FileName.c_str());  //c_str() 将string转换成char*
    if (!fout)
    {
        cerr << "Output File Name Error!" << endl;
        exit(1);
    }

    cout<<"Extract Features From: "<<Input_FilePath<<endl;

    cout<<"Processing. "<<endl;

    /*读取灰度图*/
    ReadImageToBuff(Input_FilePath.c_str(), Img, height,width,bandcount);

    GrayImg = new float[height*width];
    if (NULL == GrayImg) {
        cerr<<"Memory Error."<<endl;
        exit(1);
    }
    if (bandcount==3) {
        //Gdal_rgb2gray(Img,height,width,bandcount);
        for(i=0;i<height*width;i++) {
            GrayImg[i] = 0.212671 * Img[i] + 0.715160 * Img[i+height*width] + 0.072169 * Img[i+height*width*2];
        }
    } else if(bandcount==1) {
        for(i=0;i<height*width;i++) {
            GrayImg[i]=Img[i];
        }
    } else {
        cerr<<"Image Band Error."<<endl;
        exit(1);
    }

    //释放内存
    delete[] Img;Img=NULL;
    /*图像归一化*/
    //Normalize(GrayImg, width, height);

    vector<float> ipixels(GrayImg, GrayImg + width * height);
    //释放内存
    delete [] GrayImg;GrayImg=NULL;

    float zoom1=0;
    int wS1=0, hS1=0;
    vector<float> ipixels1_zoom;
    if (flag_resize != 0)
    {
        cout << "WARNING: The input images are resized to " << wS << "x" << hS << " for ASIFT. " << endl
             << "         But the results will be normalized to the original image size." << endl << endl;

        float InitSigma_aa = 1.6;

        float fproj_p, fproj_bg;
        char fproj_i;
        float *fproj_x4, *fproj_y4;
        int fproj_o;

        fproj_o = 3;
        fproj_p = 0;
        fproj_i = 0;
        fproj_bg = 0;
        fproj_x4 = 0;
        fproj_y4 = 0;

        float areaS = wS * hS;

        // Resize image 1
        float area1 = width * height;
        zoom1 = sqrt(area1/areaS);

        wS1 = (int) (width / zoom1);
        hS1 = (int) (height / zoom1);

        int fproj_sx = wS1;
        int fproj_sy = hS1;

        float fproj_x1 = 0;
        float fproj_y1 = 0;
        float fproj_x2 = wS1;
        float fproj_y2 = 0;
        float fproj_x3 = 0;
        float fproj_y3 = hS1;

        /* Anti-aliasing filtering along vertical direction */
        if ( zoom1 > 1 )		//缩小图像
        {
            float sigma_aa = InitSigma_aa * zoom1 / 2;
            GaussianBlur1D(ipixels,width,height,sigma_aa,1);
            GaussianBlur1D(ipixels,width,height,sigma_aa,0);
        }

        // simulate a tilt: subsample the image along the vertical axis by a factor of t.
        ipixels1_zoom.resize(wS1*hS1);
        fproj (ipixels, ipixels1_zoom, width, height, &fproj_sx, &fproj_sy, &fproj_bg, &fproj_o, &fproj_p,
               &fproj_i , fproj_x1 , fproj_y1 , fproj_x2 , fproj_y2 , fproj_x3 , fproj_y3, fproj_x4, fproj_y4);

    }
    else
    {
        ipixels1_zoom.resize(width*height);
        ipixels1_zoom = ipixels;
        wS1 = width;
        hS1 = height;
        zoom1 = 1;

    }
    //释放内存
    ipixels.clear();

    /*特征提取*/
    // Compute ASIFT keypoints
    // number N of tilts to simulate t = 1, \sqrt{2}, (\sqrt{2})^2, ..., {\sqrt{2}}^(N-1)

    int num_of_tilts = 7;
    //	int num_of_tilts1 = 1;
    //	int num_of_tilts2 = 1;
    int verb = 0;
    // Define the SIFT parameters
    siftPar siftparameters;
    default_sift_parameters(siftparameters);

    vector< vector< keypointslist > > keys;

    int num_keys=0;
    num_keys = compute_asift_keypoints(ipixels1_zoom, wS1, hS1, num_of_tilts, verb, keys, siftparameters);
    //释放内存
    ipixels1_zoom.clear();

    /*将特征写入文件*/

    int tt,rr,ii;
    for (tt = 0; tt < (int) keys.size(); tt++)
    {
        for (rr = 0; rr < (int) keys[tt].size(); rr++)
        {
            keypointslist::iterator ptr = keys[tt][rr].begin();
            for(i=0; i < (int) keys[tt][rr].size(); i++, ptr++)
            {
                //fout << zoom1*ptr->x << "  " << zoom1*ptr->y << "  " << zoom1*ptr->scale << "  " << ptr->angle;
                //fout << type[m]; //图像类别
                for (ii = 0; ii < (int) VecLength-1 ; ii++)
                {
                    fout << ptr->vec[ii] << ',';
                }
                fout << ptr->vec[VecLength];
                fout << endl;
            }
        }
    }
    //fout<<endl;

    //释放内存
    keys.clear();

    cout<<"extract done."<<endl;
    fout.close();

}

