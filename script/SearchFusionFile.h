/********************************************************************
	created:	2016/06/03
	created:	3:6:2016   19:28
	filename: 	D:\code\vs2010\C\Fusion\SearchFusionFile.h
	file path:	D:\code\vs2010\C\Fusion
	file base:	SearchFusionFile
	file ext:	h
	author:		YS
	
	purpose:	寻找满足条件的融合数据
*********************************************************************/
#include "gdal_priv.h"  //c++ 语法
#include "ogr_spatialref.h"
#include "proj_api.h"  //proj4
#include <string>
#include <math.h>
#include <time.h>
#include <iostream>
#include <vector>
#include <io.h>
using namespace std;

//判断是否为pan影像
bool isPanFile(const string filepath,vector<double> &x,vector<double> &y, float &resolution)
{
	/*isPanFile     是否Pan图像
	*filepath		图像路径
	*x				左上角，右上角，左下角，右下角
	*y				左上角，右上角，左下角，右下角
	*return			true|false
	*作者：YS
	*日期：2016.06.03
	*/

	GDALAllRegister();         //利用GDAL读取图片，先要进行注册  
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");   //设置支持中文路径 
	GDALDataset *ReadDataSet = (GDALDataset*)GDALOpen(filepath.c_str(), GA_ReadOnly);
	if(NULL==ReadDataSet){
		cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
		return -1;
	}
	int nXsize= ReadDataSet->GetRasterXSize();
	int nYsize= ReadDataSet->GetRasterYSize();
	int bandcount = ReadDataSet->GetRasterCount();

	if(bandcount!=1) 
		return false;

	double adfGeoTransform[6] = {0,0,0,0,0,0};
	if (ReadDataSet->GetGeoTransform(adfGeoTransform) == CE_None){
		resolution = abs(adfGeoTransform[1]); //空间分辨率
	}
	else{
		cout<<"resolution error"<<endl;
		return false;
	}
	
	//Proj4 
	projPJ pj_utm = NULL;
	projPJ pj_latlon = NULL;
	if(!(pj_utm = pj_init_plus("+proj=utm +zone=50 +lon_0=0 +k=1 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs"))){
		cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
		GDALClose(ReadDataSet);ReadDataSet = NULL; //释放内存
		return -1;
	}
	if(!(pj_latlon = pj_init_plus("+proj=longlat +datum=WGS84 +no_defs"))){
		cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;		
		//释放内存		
		pj_free(pj_utm);pj_utm = NULL;  
		GDALClose(ReadDataSet);ReadDataSet = NULL; 

		return -1;
	}
	cout.precision(12);
	//左上角
	double lat = adfGeoTransform[3]; //纬度
	double lon = adfGeoTransform[0]; //经度

	pj_transform(pj_utm,pj_latlon,1,1,&lon,&lat,NULL);

	x.push_back(lon/DEG_TO_RAD);	//左上角经度坐标
	y.push_back(lat/DEG_TO_RAD);    //左上角纬度坐标
	//cout<<lon/DEG_TO_RAD<<"---------"<<lat/DEG_TO_RAD<<endl;
	//右上角
	lon = adfGeoTransform[0] + nXsize * adfGeoTransform[1];
	lat = adfGeoTransform[3];
	pj_transform(pj_utm,pj_latlon,1,1,&lon,&lat,NULL);

	x.push_back(lon/DEG_TO_RAD);	//右上角经度坐标
	y.push_back(lat/DEG_TO_RAD);    //右上角纬度坐标
	//cout<<lon/DEG_TO_RAD<<"---------"<<lat/DEG_TO_RAD<<endl;
	//左下角
	lon = adfGeoTransform[0];
	lat = adfGeoTransform[3] + nYsize * adfGeoTransform[5];
	pj_transform(pj_utm,pj_latlon,1,1,&lon,&lat,NULL);

	x.push_back(lon/DEG_TO_RAD);	//左下角经度坐标
	y.push_back(lat/DEG_TO_RAD);    //左下角纬度坐标
	//cout<<lon/DEG_TO_RAD<<"---------"<<lat/DEG_TO_RAD<<endl;
	//右下角
	lat = adfGeoTransform[3] + nXsize * adfGeoTransform[4] + nYsize * adfGeoTransform[5]; //纬度
	lon = adfGeoTransform[0] + nXsize * adfGeoTransform[1] + nYsize * adfGeoTransform[2]; //经度

	pj_transform(pj_utm,pj_latlon,1,1,&lon,&lat,NULL);

	x.push_back(lon/DEG_TO_RAD);	//左上角经度坐标
	y.push_back(lat/DEG_TO_RAD);    //左上角纬度坐标
	//cout<<lon/DEG_TO_RAD<<"---------"<<lat/DEG_TO_RAD<<endl;
	
	//释放内存
	pj_free(pj_utm);pj_utm = NULL;
	pj_free(pj_latlon);pj_latlon = NULL; 
	GDALClose(ReadDataSet);ReadDataSet = NULL;
	return true;
}

//判断是否为ms影像
bool isMsFile(const string filepath,vector<double> &x,vector<double> &y, float &resolution)
{
	/*isMsFile     是否MS图像
	*filepath		图像路径
	*x				左上角，右上角，左下角，右下角
	*y				左上角，右上角，左下角，右下角
	*return			true|false
	*作者：YS
	*日期：2016.06.03
	*/
	GDALAllRegister();         //利用GDAL读取图片，先要进行注册  
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");   //设置支持中文路径 
	GDALDataset *ReadDataSet = (GDALDataset*)GDALOpen(filepath.c_str(), GA_ReadOnly);
	if(NULL==ReadDataSet){
		cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
		return -1;
	}
	int nXsize= ReadDataSet->GetRasterXSize();
	int nYsize= ReadDataSet->GetRasterYSize();
	int bandcount = ReadDataSet->GetRasterCount();

	if(bandcount == 1) 
		return false;

	double adfGeoTransform[6] = {0,0,0,0,0,0};
	if (ReadDataSet->GetGeoTransform(adfGeoTransform) == CE_None){
		resolution = abs(adfGeoTransform[1]); //空间分辨率
	}
	else{
		cout<<"resolution error"<<endl;
		return false;
	}

	//Proj4 
	projPJ pj_utm = NULL;
	projPJ pj_latlon = NULL;
	if(!(pj_utm = pj_init_plus("+proj=utm +zone=50 +lon_0=0 +k=1 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs"))){
		cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
		GDALClose(ReadDataSet);ReadDataSet = NULL; //释放内存
		return -1;
	}
	if(!(pj_latlon = pj_init_plus("+proj=longlat +datum=WGS84 +no_defs"))){
		cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;		
		//释放内存		
		pj_free(pj_utm);pj_utm = NULL;  
		GDALClose(ReadDataSet);ReadDataSet = NULL; 

		return -1;
	}
	
	cout.precision(12);
	//左上角
	double lat = adfGeoTransform[3]; //纬度
	double lon = adfGeoTransform[0]; //经度

	pj_transform(pj_utm,pj_latlon,1,1,&lon,&lat,NULL);

	x.push_back(lon/DEG_TO_RAD);	//左上角经度坐标
	y.push_back(lat/DEG_TO_RAD);    //左上角纬度坐标
	//cout<<lon/DEG_TO_RAD<<"---------"<<lat/DEG_TO_RAD<<endl;
	//右上角
	lon = adfGeoTransform[0] + nXsize * adfGeoTransform[1];
	lat = adfGeoTransform[3];
	pj_transform(pj_utm,pj_latlon,1,1,&lon,&lat,NULL);

	x.push_back(lon/DEG_TO_RAD);	//右上角经度坐标
	y.push_back(lat/DEG_TO_RAD);    //右上角纬度坐标
	//cout<<lon/DEG_TO_RAD<<"---------"<<lat/DEG_TO_RAD<<endl;
	//左下角
	lon = adfGeoTransform[0];
	lat = adfGeoTransform[3] + nYsize * adfGeoTransform[5];
	pj_transform(pj_utm,pj_latlon,1,1,&lon,&lat,NULL);

	x.push_back(lon/DEG_TO_RAD);	//左下角经度坐标
	y.push_back(lat/DEG_TO_RAD);    //左下角纬度坐标
	//cout<<lon/DEG_TO_RAD<<"---------"<<lat/DEG_TO_RAD<<endl;
	//右下角
	lat = adfGeoTransform[3] + nXsize * adfGeoTransform[4] + nYsize * adfGeoTransform[5]; //纬度
	lon = adfGeoTransform[0] + nXsize * adfGeoTransform[1] + nYsize * adfGeoTransform[2]; //经度

	pj_transform(pj_utm,pj_latlon,1,1,&lon,&lat,NULL);

	x.push_back(lon/DEG_TO_RAD);	//左上角经度坐标
	y.push_back(lat/DEG_TO_RAD);    //左上角纬度坐标
	//cout<<lon/DEG_TO_RAD<<"---------"<<lat/DEG_TO_RAD<<endl;


	//释放内存
	pj_free(pj_utm);pj_utm = NULL;
	pj_free(pj_latlon);pj_latlon = NULL; 
	GDALClose(ReadDataSet);ReadDataSet = NULL;
	return true;

}

bool SearchFusionFile(const string filepath, const string output)
{
	/*SearchFusionFile      配对
	*filepath				图像路径
	*return					true|false
	*作者：YS
	*日期：2016.06.03
	*/
	_finddata_t PanFile;
	_finddata_t MsFile;
	long lpan;
	long lms;
	//写入文件
	ofstream fout(output.c_str());  
	if (!fout)  
	{  
		cerr << "output error!" << endl;  
		return false;  
	}  

	//配对
	if((lpan=_findfirst((filepath+"\\*.*").c_str(),&PanFile))==-1)
	{
		cerr<<filepath<<"Not Found!"<<endl;
		return false;
	}
	else{

		_findnext(lpan,&PanFile);
		while(_findnext(lpan,&PanFile)==0) //获取所有文件
		{
			string panimgname=PanFile.name; //文件名称
			int comma=panimgname.find_last_of(".");
			if (panimgname.substr(comma+1,panimgname.length()) != "tif" )	continue;
			
			string panimgfilepath=filepath+"\\"+panimgname; //路径拼接
			vector<double> panx;
			vector<double> pany;
			float panresolution;
			if (!isPanFile(panimgfilepath, panx, pany, panresolution)) continue;
			//cout<<panresolution<<endl;
			if((lms=_findfirst((filepath+"\\*.*").c_str(),&MsFile))==-1)
			{
				cerr<<filepath<<"Not Found!"<<endl;
				return false;
			}
			_findnext(lms,&MsFile);
			while(_findnext(lms,&MsFile)==0)//获取所有文件
			{
				string msimgname=MsFile.name; //文件名称
				int mscomma=msimgname.find_last_of(".");
				if (msimgname.substr(mscomma+1,msimgname.length()) != "tif" )	continue;

				string msimgfilepath=filepath+"\\"+msimgname; //路径拼接
				vector<double> msx;
				vector<double> msy;
				float msresolution;
				if (!isMsFile(msimgfilepath, msx, msy, msresolution)) continue;
				//cout<<msresolution<<endl;
				if(msresolution/panresolution>4) continue;

				if ((msx[0]-0.001<=panx[0]&&panx[0]<=msx[3]+0.001)&&(msy[3]-0.001<=pany[0]&&pany[0]<=msy[0]+0.001)||
					(msx[0]-0.001<=panx[1]&&panx[1]<=msx[3]+0.001)&&(msy[3]-0.001<=pany[1]&&pany[1]<=msy[0]+0.001)||
					(msx[0]-0.001<=panx[2]&&panx[2]<=msx[3]+0.001)&&(msy[3]-0.001<=pany[2]&&pany[2]<=msy[0]+0.001)||
					(msx[0]-0.001<=panx[3]&&panx[3]<=msx[3]+0.001)&&(msy[3]-0.001<=pany[3]&&pany[3]<=msy[0]+0.001)||
					(panx[0]-0.001<=msx[0]&&msx[0]<=panx[3]+0.001)&&(pany[3]-0.001<=msy[0]&&msy[0]<=pany[0]+0.001)||
					(panx[0]-0.001<=msx[1]&&msx[1]<=panx[3]+0.001)&&(pany[3]-0.001<=msy[1]&&msy[1]<=pany[0]+0.001)||
					(panx[0]-0.001<=msx[2]&&msx[2]<=panx[3]+0.001)&&(pany[3]-0.001<=msy[2]&&msy[2]<=pany[0]+0.001)||
					(panx[0]-0.001<=msx[3]&&msx[3]<=panx[3]+0.001)&&(pany[3]-0.001<=msy[3]&&msy[3]<=pany[0]+0.001))
				{
					cout<<"find:"<<endl;
					cout<<panimgfilepath<<endl;
					cout<<msimgfilepath<<endl;
					fout<<panimgfilepath<<","<<msimgfilepath<<endl;
				}

			}

		}
	}
}