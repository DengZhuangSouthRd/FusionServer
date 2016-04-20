#ifndef _PGINF_H_
#define _PGINF_H_
#include "gdal_priv.h"  //c++ 语法
#include "ogr_spatialref.h"
#include "proj_api.h"  //proj4
#include <string>
#include <math.h>
#include <time.h>
#include <iostream>
using namespace std;

struct FusionInf{
    //数据库写入信息
    double	cnttimeuse;				//所用时间
    string	producetime;			//产品生成时间
    double	ulcoorvalid_longitude;	//有效融合区域左上角经度坐标
    double	ulcoorvalid_latitude ;	//有效融合区域左上角纬度坐标
    double	brcoorvalid_longitude;	//有效融合区域右下角经度坐标
    double	brcoorvalid_latitude ;	//有效融合区域右下角纬度坐标
    double	ulcoorwhole_longitude;	//全局左上角经度坐标
    double	ulcoorwhole_latitude ;	//全局左上角纬度坐标
    double	brcoorwhole_longitude;	//全局右下角经度坐标
    double	brcoorwhole_latitude ;	//全局右下角纬度坐标
    float	resolution;				//融合后产品分辨率
    string	product_format;			//产品格式
    string	projectiontype;			//投影类型
    string	datumname;				//投影椭球体
    string	projectioncode;			//投影编码
    string	projectionunits;		//投影单位
    float	projcentralmeridian;	//投影带中央经线
};

class PgInf{
private:
    FusionInf finf;
public:
    PgInf(){
        //初始化
        finf.cnttimeuse = 0;				//所用时间
        finf.producetime = "";			//产品生成时间
        finf.ulcoorvalid_longitude = 0;	//有效融合区域左上角经度坐标
        finf.ulcoorvalid_latitude  = 0;	//有效融合区域左上角纬度坐标
        finf.brcoorvalid_longitude = 0;	//有效融合区域右下角经度坐标
        finf.brcoorvalid_latitude  = 0;	//有效融合区域右下角纬度坐标
        finf.ulcoorwhole_longitude = 0;	//全局左上角经度坐标
        finf.ulcoorwhole_latitude  = 0;	//全局左上角纬度坐标
        finf.brcoorwhole_longitude = 0;	//全局右下角经度坐标
        finf.brcoorwhole_latitude  = 0;	//全局右下角纬度坐标
        finf.resolution = 0;				//融合后产品分辨率
        finf.product_format = "";		//产品格式
        finf.projectiontype = "";		//投影类型
        finf.datumname = "";				//投影椭球体
        finf.projectioncode = "";		//投影编码
        finf.projectionunits = "";		//投影单位
        finf.projcentralmeridian =0;		//投影带中央经线
    }

    FusionInf GetInf(){
        return this->finf;
    }

    inline void SetCnttimeuse(double Cnttimeuse){
        this->finf.cnttimeuse = Cnttimeuse;
    }

    inline void SetProducetime(string Producetime){

        this->finf.producetime=Producetime;

    }

    int SetProductInf(string Outrl){
        /*SetProductInf 解析融合产品，获取产品相关信息
        *OutUrl			融合产品路径
        *作者：YS
        *日期：2016.04.12
        */

        GDALAllRegister();         //利用GDAL读取图片，先要进行注册
        CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");   //设置支持中文路径
        GDALDataset *ReadDataSet = (GDALDataset*)GDALOpen(Outrl.c_str(), GA_ReadOnly);
        if(NULL==ReadDataSet){
            cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
            return -1;
        }

        double adfGeoTransform[6] = {0,0,0,0,0,0};
        if (ReadDataSet->GetGeoTransform(adfGeoTransform) == CE_None)
        {
            this->finf.resolution = fabs(adfGeoTransform[1]); //空间分辨率
        }
        else
        {
            this->finf.resolution = 0; //空间分辨率
        }
        cout<<"resolution:"<<this->finf.resolution<<endl;

        int nXsize= ReadDataSet->GetRasterXSize();
        int nYsize= ReadDataSet->GetRasterYSize();

        //Proj4
        //2016.04.11
        projPJ pj_utm, pj_latlon;
        if(!(pj_utm = pj_init_plus("+proj=utm +zone=50 +lon_0=0 +k=1 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs"))){
            cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
            return -1;
        }
        if(!(pj_latlon = pj_init_plus("+proj=longlat +datum=WGS84 +no_defs"))){
            cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
            return -1;
        }

        //左上角
        //double lat = adfGeoTransform[3]; //纬度
        //double lon = adfGeoTransform[0]; //经度
        double lat = adfGeoTransform[3] + 0.5 * adfGeoTransform[4] + 0.5 * adfGeoTransform[5]; //纬度
        double lon = adfGeoTransform[0] + 0.5 * adfGeoTransform[1] + 0.5 * adfGeoTransform[2]; //经度

        pj_transform(pj_utm,pj_latlon,1,1,&lon,&lat,NULL);

        cout.precision(12);
        cout<<"lon:"<<lon/DEG_TO_RAD<<" lat:"<<lat/DEG_TO_RAD<<endl;

        this->finf.ulcoorvalid_longitude = lon/DEG_TO_RAD;	//有效融合区域左上角经度坐标
        this->finf.ulcoorvalid_latitude = lat/DEG_TO_RAD;	//有效融合区域左上角纬度坐标
        this->finf.ulcoorwhole_longitude = lon/DEG_TO_RAD;	//全局左上角经度坐标
        this->finf.ulcoorwhole_latitude = lat/DEG_TO_RAD;	//全局左上角纬度坐标

        //右下角
        //lat = adfGeoTransform[3] + nXsize * adfGeoTransform[4] + nYsize * adfGeoTransform[5]; //纬度
        //lon = adfGeoTransform[0] + nXsize * adfGeoTransform[1] + nYsize * adfGeoTransform[2]; //经度
        lat = adfGeoTransform[3] + (nXsize-0.5) * adfGeoTransform[4] + (nYsize-0.5) * adfGeoTransform[5]; //纬度
        lon = adfGeoTransform[0] + (nXsize-0.5) * adfGeoTransform[1] + (nYsize-0.5) * adfGeoTransform[2]; //经度

        pj_transform(pj_utm,pj_latlon,1,1,&lon,&lat,NULL);

        cout<<"lon:"<<lon/DEG_TO_RAD<<" lat:"<<lat/DEG_TO_RAD<<endl;
        this->finf.brcoorvalid_longitude = lon/DEG_TO_RAD;		//有效融合区域右下角经度坐标
        this->finf.brcoorvalid_latitude = lat/DEG_TO_RAD;		//有效融合区域右下角纬度坐标
        this->finf.brcoorwhole_longitude = lon/DEG_TO_RAD;		//全局右下角经度坐标
        this->finf.brcoorwhole_latitude = lat/DEG_TO_RAD;		//全局右下角纬度坐标


        //其他信息
        this->finf.product_format = "GEOTIFF";		//产品格式
        this->finf.projectiontype = "WGS84";			//投影类型
        this->finf.datumname = "WGS84";				//投影椭球体
        this->finf.projectioncode = "WGS84";			//投影编码
        this->finf.projectionunits = "WGS84";		//投影单位
        this->finf.projcentralmeridian = 0;			//投影带中央经线

        GDALClose(ReadDataSet);ReadDataSet = NULL;
    }

    //融合产品信息写入数据库
    int ReadInfToDB(double Cnttimeuse,string OutUrl,string Producetime){
        /*ReadInfToDB 待写入数据库的相关信息
        *Cnttimeuse		融合算法所用时间
        *OutUrl			融合产品路径
        *Producetime	融合产品生产时间
        *作者：YS
        *日期：2016.04.12
        */
        this->SetCnttimeuse(Cnttimeuse);
        this->SetProducetime(Producetime);
        this->SetProductInf(OutUrl);

        return 0;
    }

    ~PgInf() {}
};


#endif
