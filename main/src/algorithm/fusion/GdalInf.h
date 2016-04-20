/********************************************************************
    created:	2015/12/07
    created:	7:12:2015   19:36
    filename: 	D:\code\vs2010\C\Fusion\IO.h
    file path:	D:\code\vs2010\C\Fusion
    file base:	GdalInf
    file ext:	h
    author:		YS

    purpose:	GdalInf类
                处理遥感影像读取、写入的相关信息
*********************************************************************/
#ifndef _GDALINF_H_
#define _GDALINF_H_

#include "gdal_priv.h"  //c++ 语法
#include "ogr_spatialref.h"
#include "gdalwarper.h"
#include <cassert>
#include <string.h>
#include <iostream>
using namespace std;

typedef unsigned short int ushort;


class Inf {
protected:
    //图像基本信息
    int height;					// 行数
    int width;					// 列数
    int bandcount;				//波段数
    /* 待拓展图像信息 */
    float SpatialSolution;      //空间分辨率
    const char* Description;    //描述信息 图像格式
    const char* ProjectionRef;	//投影信息
    double adfGeoTransform[6];  //地理坐标信息
    const char * Datatype;
public:
    inline int GetHeight(){	//获取图像高度
        return this->height;
    }

    inline void SetHeight(int NewHeight){ //设置图像高度
        this->height = NewHeight;
    }

    inline int GetWidth(){		//获取图像宽度
        return this->width;
    }
    inline void SetWidth(int NewWidth){  //设置图像宽度
        this->width = NewWidth;
    }

    inline int GetBandCount(){		//获取图像波段数
        return this->bandcount;
    }
    inline void SetBandCount(int nband){		//设置图像波段数
        this->bandcount = nband;
    }
    inline float GetSpatialSolution(){		//获取图像空间分辨率
        return this->SpatialSolution;
    }
    inline void SetSpatialSolution(int NewSpatialSolution){		//设置图像空间分辨率
        this->SpatialSolution = NewSpatialSolution;
    }
    inline double * GetGeoTransform(){		//获取图像地理信息 2016.3.14
        return this->adfGeoTransform;
        //memcpy(this->adfGeoTransform,newGeoTransform,sizeof(double)*6);
    }
    inline void SetGeoTransform(double *newGeoTransform){		//获取图像地理信息 2016.3.14

        memcpy(this->adfGeoTransform,newGeoTransform,sizeof(double)*6);
    }
};

class GdalInf :public Inf{

private:
    float * imgdata;			// 指向存放元素的空间
public:
    //构造函数，指针初始化为空
    GdalInf(){
        //初始化
        height = 0;					// 行数
        width = 0;					// 列数
        bandcount = 0;				//波段数
        SpatialSolution = 0;        //空间分辨率
        Description = NULL;    //描述信息 图像格式
        ProjectionRef = NULL;	//投影信息
        adfGeoTransform[0] = 0;  //左上角像元的东坐标
        adfGeoTransform[1] = 0;  //宽度上的分辨率
        adfGeoTransform[2] = 0;  //旋转，0表示上面为北方
        adfGeoTransform[3] = 0;  //左上角像元的北坐标
        adfGeoTransform[4] = 0;  //旋转，0表示上面为北方
        adfGeoTransform[5] = 0;  //高度上的分辨率
        Datatype = NULL;
        this->imgdata = NULL;
    }

    //获取图像指针
    float * GetImgData(){
        if (NULL == this->imgdata)
        {
            cerr<<"Imgdata is null.\n";
            cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
            exit(1);
        }
        return this->imgdata;
    }

    //图像数据
    void SetImgData(float* NewData){
        this->imgdata = NewData;
    }

    //释放图像内存
    void ClearImageData(){
        if (NULL != this->imgdata)
        {
            delete[] this->imgdata;
            this->imgdata = NULL;
        }
    }

    //Gdal写图像信息
    int WriteImageInf(const char* OutputFileName)
    {
        /*
        * @brief    WriteImageInf.
        * 把影像写入到文件
        * @param    OutputFileName      输出文件路径
        * @return   0   成功
        * @return   -1  内存分配失败
        */


        const char *GType = "GTIFF";

        GDALDriver *pDstDriver = NULL;
        pDstDriver = GetGDALDriverManager()->GetDriverByName(GType);
        if (NULL == pDstDriver) {
            cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
            return -1;
        }

        GDALDataset * pDstDataSet = pDstDriver->Create(OutputFileName, this->width, this->height, this->bandcount, GDT_UInt16, NULL);


        //写头文件
        char *WKT = NULL;
        OGRSpatialReference oSRS;
        oSRS.SetWellKnownGeogCS("WGS84");
        oSRS.SetUTM(50,TRUE);
        oSRS.exportToWkt(&WKT);
        //oSRS1.importFromWkt(&WKT);
        pDstDataSet->SetProjection(WKT);
        CPLFree(WKT);

        pDstDataSet->SetGeoTransform(this->adfGeoTransform);


        pDstDriver = NULL;
        GDALClose(pDstDataSet);pDstDataSet=NULL;

        return 0;
    }

    //Gdal写图像
    int WriteImageFromBuff(const char* OutputFileName, int x,int y,int NewWidth,int NewHeight)
    {
        /*
        * @brief    WriteImageFromBuff.
        * 把影像写入到文件
        * @param    OutputFileName      输出文件路径
        * @param    （x，y）			写入图像起始坐标
        * @param    NewWidth	     	写入的宽度
        * @param    NewHeight	    	写入的高度
        * @return   0   成功
        * @return   -1  内存分配失败
        */
        GDALDataset *WriteDataSet = (GDALDataset*)GDALOpen(OutputFileName, GA_Update);
        ushort* img=new ushort[NewWidth*NewHeight*this->bandcount];

        if(NULL == WriteDataSet||NULL == img||NULL == imgdata){
            cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
            return -1;
        }
        //if (NULL == imgdata)
        //{
        //	cout<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
        //	return -1;
        //}
        int i;
        for (i=0;i<NewWidth*NewHeight*this->bandcount;i++)
            img[i]=(ushort)imgdata[i] > 0 ? (ushort)imgdata[i] : 0; //截断

        if (WriteDataSet->RasterIO(GF_Write, x, y, NewWidth, NewHeight, img,NewWidth, NewHeight, GDT_UInt16, bandcount, NULL, 0, 0, 0) == CE_Failure)
        {
            delete WriteDataSet; WriteDataSet = NULL;
            delete[] imgdata; imgdata = NULL;
            cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
            return -2;
        }
        delete[] img;img=NULL;
        delete WriteDataSet; WriteDataSet = NULL;

        return 0;
    }

    //Gdal读图像信息
    int ReadImageInf(const char* InputFileName) {
        /*
        * @brief    ReadImageInf.
        * 把影像读入到内存空间
        * @param    InputFileName       输入文件路径
        * @return   0   成功
        * @return   -1  错误
        */

        //准备读取图片
        GDALDataset *ReadDataSet = (GDALDataset*)GDALOpen(InputFileName, GA_ReadOnly);
        if(NULL==ReadDataSet) {
            cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
            return -1;
        }
        //读取信息
        this->width = ReadDataSet->GetRasterXSize();	//图像宽度
        this->height = ReadDataSet->GetRasterYSize();	//图像高度
        this->bandcount = ReadDataSet->GetRasterCount();//图像波段数
        if (this->width<=0||this->height<=0||this->bandcount<=0){
            cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
            return -1;
        }
        //扩展
        this->Description = ReadDataSet->GetDescription();//获取图像的描述信息
        this->ProjectionRef = ReadDataSet->GetProjectionRef();//获取图像的投影信息
        this->Datatype = GDALGetDataTypeName(ReadDataSet->GetRasterBand(1)->GetRasterDataType()); //获取图像数据类型
        if (ReadDataSet->GetGeoTransform(this->adfGeoTransform) == CE_None) //获取图像地理信息
        {
            this->SpatialSolution = fabs(adfGeoTransform[1]); //空间分辨率

        }
        else
        {
            this->SpatialSolution = 0; //空间分辨率

        }


        delete ReadDataSet; ReadDataSet = NULL;
        return 0;
    }

    //Gdal读图像
    int ReadImageToBuff(const char* InputFileName,int x,int y,int NewWidth,int NewHeight)
    {
        /*
        * @brief    ReadImageToBuff.
        * 把影像读入到内存空间
        * @param    InputFileName		输入文件路径
        * @param    （x，y）			读取图像起始坐标
        * @param    NewWidth	     	读取的宽度
        * @param    NewHeight	    	读取的高度
        * @return   0   成功
        * @return   -1  内存分配失败
        * @return   -2  读取数据失败
        */

        //准备读取图片
        if (x+NewWidth > this->width||y+NewHeight > this->height)
        {
            cerr<<"Param Error.\n";
            cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
            return -1;
        }

        GDALDataset *ReadDataSet = (GDALDataset*)GDALOpen(InputFileName, GA_ReadOnly);
        if(NULL==ReadDataSet){return -1;}
        //
        this->ClearImageData();
        imgdata = new float[NewWidth*NewHeight*this->bandcount];

        if (NULL == imgdata)
        {
            delete ReadDataSet; ReadDataSet = NULL;
            cerr<<"Memory Error.\n";
            cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
            return -1;
        }

        if (ReadDataSet->RasterIO(GF_Read, x, y, NewWidth, NewHeight, imgdata, NewWidth, NewHeight, GDT_Float32, bandcount, NULL, 0, 0, 0) == CE_Failure)
        {
            delete ReadDataSet; ReadDataSet = NULL;
            delete[] imgdata; imgdata = NULL;
            cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
            return -2;
        }
        delete ReadDataSet; ReadDataSet = NULL;

        return 0;
    }

    int ReadImageToBuff(const char* InputFileName,int x,int y,int NewWidth,int NewHeight,int *bandlist)
    {
        /*
        * @brief    ReadImageToBuff.
        * 把影像读入到内存空间
        * @param    InputFileName		输入文件路径
        * @param    （x，y）			读取图像起始坐标
        * @param    NewWidth	     	读取的宽度
        * @param    NewHeight	    	读取的高度
        * @param    bandlist			读取波段顺序
        * @return   0   成功
        * @return   -1  内存分配失败
        * @return   -2  读取数据失败
        */

        //准备读取图片
        if (x+NewWidth > this->width||y+NewHeight > this->height)
        {
            cerr<<"Param Error.\n";
            cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
            return -1;
        }

        GDALDataset *ReadDataSet = (GDALDataset*)GDALOpen(InputFileName, GA_ReadOnly);
        if(NULL==ReadDataSet){return -1;}
        //
        this->ClearImageData();
        imgdata = new float[NewWidth*NewHeight*this->bandcount];

        if (NULL == imgdata)
        {
            delete ReadDataSet; ReadDataSet = NULL;
            cerr<<"Memory Error.\n";
            cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
            return -1;
        }

        if (ReadDataSet->RasterIO(GF_Read, x, y, NewWidth, NewHeight, imgdata, NewWidth, NewHeight, GDT_Float32, bandcount, bandlist, 0, 0, 0) == CE_Failure)
        {
            delete ReadDataSet; ReadDataSet = NULL;
            delete[] imgdata; imgdata = NULL;
            cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
            return -2;
        }
        delete ReadDataSet; ReadDataSet = NULL;

        return 0;
    }


    ~GdalInf(){
        this->ClearImageData();
    }
};


#endif
