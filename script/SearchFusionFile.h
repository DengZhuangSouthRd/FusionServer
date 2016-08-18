/********************************************************************
	created:	2016/06/03
	created:	3:6:2016   19:28
	filename: 	D:\code\vs2010\C\Fusion\SearchFusionFile.h
	file path:	D:\code\vs2010\C\Fusion
	file base:	SearchFusionFile
	file ext:	h
	author:		YS
	
	purpose:	Ѱ�������������ں�����
*********************************************************************/
#include "gdal_priv.h"  //c++ �﷨
#include "ogr_spatialref.h"
#include "proj_api.h"  //proj4
#include <string>
#include <math.h>
#include <time.h>
#include <iostream>
#include <vector>
#include <io.h>
using namespace std;

//�ж��Ƿ�ΪpanӰ��
bool isPanFile(const string filepath,vector<double> &x,vector<double> &y, float &resolution)
{
	/*isPanFile     �Ƿ�Panͼ��
	*filepath		ͼ��·��
	*x				���Ͻǣ����Ͻǣ����½ǣ����½�
	*y				���Ͻǣ����Ͻǣ����½ǣ����½�
	*return			true|false
	*���ߣ�YS
	*���ڣ�2016.06.03
	*/

	GDALAllRegister();         //����GDAL��ȡͼƬ����Ҫ����ע��  
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");   //����֧������·�� 
	GDALDataset *ReadDataSet = (GDALDataset*)GDALOpen(filepath.c_str(), GA_ReadOnly);
	if(NULL==ReadDataSet){
		cerr<<"file��"<<__FILE__<<"line��"<<__LINE__<<"time��"<<__DATE__<<" "<<__TIME__<<endl;
		return -1;
	}
	int nXsize= ReadDataSet->GetRasterXSize();
	int nYsize= ReadDataSet->GetRasterYSize();
	int bandcount = ReadDataSet->GetRasterCount();

	if(bandcount!=1) 
		return false;

	double adfGeoTransform[6] = {0,0,0,0,0,0};
	if (ReadDataSet->GetGeoTransform(adfGeoTransform) == CE_None){
		resolution = abs(adfGeoTransform[1]); //�ռ�ֱ���
	}
	else{
		cout<<"resolution error"<<endl;
		return false;
	}
	
	//Proj4 
	projPJ pj_utm = NULL;
	projPJ pj_latlon = NULL;
	if(!(pj_utm = pj_init_plus("+proj=utm +zone=50 +lon_0=0 +k=1 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs"))){
		cerr<<"file��"<<__FILE__<<"line��"<<__LINE__<<"time��"<<__DATE__<<" "<<__TIME__<<endl;
		GDALClose(ReadDataSet);ReadDataSet = NULL; //�ͷ��ڴ�
		return -1;
	}
	if(!(pj_latlon = pj_init_plus("+proj=longlat +datum=WGS84 +no_defs"))){
		cerr<<"file��"<<__FILE__<<"line��"<<__LINE__<<"time��"<<__DATE__<<" "<<__TIME__<<endl;		
		//�ͷ��ڴ�		
		pj_free(pj_utm);pj_utm = NULL;  
		GDALClose(ReadDataSet);ReadDataSet = NULL; 

		return -1;
	}
	cout.precision(12);
	//���Ͻ�
	double lat = adfGeoTransform[3]; //γ��
	double lon = adfGeoTransform[0]; //����

	pj_transform(pj_utm,pj_latlon,1,1,&lon,&lat,NULL);

	x.push_back(lon/DEG_TO_RAD);	//���ϽǾ�������
	y.push_back(lat/DEG_TO_RAD);    //���Ͻ�γ������
	//cout<<lon/DEG_TO_RAD<<"---------"<<lat/DEG_TO_RAD<<endl;
	//���Ͻ�
	lon = adfGeoTransform[0] + nXsize * adfGeoTransform[1];
	lat = adfGeoTransform[3];
	pj_transform(pj_utm,pj_latlon,1,1,&lon,&lat,NULL);

	x.push_back(lon/DEG_TO_RAD);	//���ϽǾ�������
	y.push_back(lat/DEG_TO_RAD);    //���Ͻ�γ������
	//cout<<lon/DEG_TO_RAD<<"---------"<<lat/DEG_TO_RAD<<endl;
	//���½�
	lon = adfGeoTransform[0];
	lat = adfGeoTransform[3] + nYsize * adfGeoTransform[5];
	pj_transform(pj_utm,pj_latlon,1,1,&lon,&lat,NULL);

	x.push_back(lon/DEG_TO_RAD);	//���½Ǿ�������
	y.push_back(lat/DEG_TO_RAD);    //���½�γ������
	//cout<<lon/DEG_TO_RAD<<"---------"<<lat/DEG_TO_RAD<<endl;
	//���½�
	lat = adfGeoTransform[3] + nXsize * adfGeoTransform[4] + nYsize * adfGeoTransform[5]; //γ��
	lon = adfGeoTransform[0] + nXsize * adfGeoTransform[1] + nYsize * adfGeoTransform[2]; //����

	pj_transform(pj_utm,pj_latlon,1,1,&lon,&lat,NULL);

	x.push_back(lon/DEG_TO_RAD);	//���ϽǾ�������
	y.push_back(lat/DEG_TO_RAD);    //���Ͻ�γ������
	//cout<<lon/DEG_TO_RAD<<"---------"<<lat/DEG_TO_RAD<<endl;
	
	//�ͷ��ڴ�
	pj_free(pj_utm);pj_utm = NULL;
	pj_free(pj_latlon);pj_latlon = NULL; 
	GDALClose(ReadDataSet);ReadDataSet = NULL;
	return true;
}

//�ж��Ƿ�ΪmsӰ��
bool isMsFile(const string filepath,vector<double> &x,vector<double> &y, float &resolution)
{
	/*isMsFile     �Ƿ�MSͼ��
	*filepath		ͼ��·��
	*x				���Ͻǣ����Ͻǣ����½ǣ����½�
	*y				���Ͻǣ����Ͻǣ����½ǣ����½�
	*return			true|false
	*���ߣ�YS
	*���ڣ�2016.06.03
	*/
	GDALAllRegister();         //����GDAL��ȡͼƬ����Ҫ����ע��  
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");   //����֧������·�� 
	GDALDataset *ReadDataSet = (GDALDataset*)GDALOpen(filepath.c_str(), GA_ReadOnly);
	if(NULL==ReadDataSet){
		cerr<<"file��"<<__FILE__<<"line��"<<__LINE__<<"time��"<<__DATE__<<" "<<__TIME__<<endl;
		return -1;
	}
	int nXsize= ReadDataSet->GetRasterXSize();
	int nYsize= ReadDataSet->GetRasterYSize();
	int bandcount = ReadDataSet->GetRasterCount();

	if(bandcount == 1) 
		return false;

	double adfGeoTransform[6] = {0,0,0,0,0,0};
	if (ReadDataSet->GetGeoTransform(adfGeoTransform) == CE_None){
		resolution = abs(adfGeoTransform[1]); //�ռ�ֱ���
	}
	else{
		cout<<"resolution error"<<endl;
		return false;
	}

	//Proj4 
	projPJ pj_utm = NULL;
	projPJ pj_latlon = NULL;
	if(!(pj_utm = pj_init_plus("+proj=utm +zone=50 +lon_0=0 +k=1 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs"))){
		cerr<<"file��"<<__FILE__<<"line��"<<__LINE__<<"time��"<<__DATE__<<" "<<__TIME__<<endl;
		GDALClose(ReadDataSet);ReadDataSet = NULL; //�ͷ��ڴ�
		return -1;
	}
	if(!(pj_latlon = pj_init_plus("+proj=longlat +datum=WGS84 +no_defs"))){
		cerr<<"file��"<<__FILE__<<"line��"<<__LINE__<<"time��"<<__DATE__<<" "<<__TIME__<<endl;		
		//�ͷ��ڴ�		
		pj_free(pj_utm);pj_utm = NULL;  
		GDALClose(ReadDataSet);ReadDataSet = NULL; 

		return -1;
	}
	
	cout.precision(12);
	//���Ͻ�
	double lat = adfGeoTransform[3]; //γ��
	double lon = adfGeoTransform[0]; //����

	pj_transform(pj_utm,pj_latlon,1,1,&lon,&lat,NULL);

	x.push_back(lon/DEG_TO_RAD);	//���ϽǾ�������
	y.push_back(lat/DEG_TO_RAD);    //���Ͻ�γ������
	//cout<<lon/DEG_TO_RAD<<"---------"<<lat/DEG_TO_RAD<<endl;
	//���Ͻ�
	lon = adfGeoTransform[0] + nXsize * adfGeoTransform[1];
	lat = adfGeoTransform[3];
	pj_transform(pj_utm,pj_latlon,1,1,&lon,&lat,NULL);

	x.push_back(lon/DEG_TO_RAD);	//���ϽǾ�������
	y.push_back(lat/DEG_TO_RAD);    //���Ͻ�γ������
	//cout<<lon/DEG_TO_RAD<<"---------"<<lat/DEG_TO_RAD<<endl;
	//���½�
	lon = adfGeoTransform[0];
	lat = adfGeoTransform[3] + nYsize * adfGeoTransform[5];
	pj_transform(pj_utm,pj_latlon,1,1,&lon,&lat,NULL);

	x.push_back(lon/DEG_TO_RAD);	//���½Ǿ�������
	y.push_back(lat/DEG_TO_RAD);    //���½�γ������
	//cout<<lon/DEG_TO_RAD<<"---------"<<lat/DEG_TO_RAD<<endl;
	//���½�
	lat = adfGeoTransform[3] + nXsize * adfGeoTransform[4] + nYsize * adfGeoTransform[5]; //γ��
	lon = adfGeoTransform[0] + nXsize * adfGeoTransform[1] + nYsize * adfGeoTransform[2]; //����

	pj_transform(pj_utm,pj_latlon,1,1,&lon,&lat,NULL);

	x.push_back(lon/DEG_TO_RAD);	//���ϽǾ�������
	y.push_back(lat/DEG_TO_RAD);    //���Ͻ�γ������
	//cout<<lon/DEG_TO_RAD<<"---------"<<lat/DEG_TO_RAD<<endl;


	//�ͷ��ڴ�
	pj_free(pj_utm);pj_utm = NULL;
	pj_free(pj_latlon);pj_latlon = NULL; 
	GDALClose(ReadDataSet);ReadDataSet = NULL;
	return true;

}

bool SearchFusionFile(const string filepath, const string output)
{
	/*SearchFusionFile      ���
	*filepath				ͼ��·��
	*return					true|false
	*���ߣ�YS
	*���ڣ�2016.06.03
	*/
	_finddata_t PanFile;
	_finddata_t MsFile;
	long lpan;
	long lms;
	//д���ļ�
	ofstream fout(output.c_str());  
	if (!fout)  
	{  
		cerr << "output error!" << endl;  
		return false;  
	}  

	//���
	if((lpan=_findfirst((filepath+"\\*.*").c_str(),&PanFile))==-1)
	{
		cerr<<filepath<<"Not Found!"<<endl;
		return false;
	}
	else{

		_findnext(lpan,&PanFile);
		while(_findnext(lpan,&PanFile)==0) //��ȡ�����ļ�
		{
			string panimgname=PanFile.name; //�ļ�����
			int comma=panimgname.find_last_of(".");
			if (panimgname.substr(comma+1,panimgname.length()) != "tif" )	continue;
			
			string panimgfilepath=filepath+"\\"+panimgname; //·��ƴ��
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
			while(_findnext(lms,&MsFile)==0)//��ȡ�����ļ�
			{
				string msimgname=MsFile.name; //�ļ�����
				int mscomma=msimgname.find_last_of(".");
				if (msimgname.substr(mscomma+1,msimgname.length()) != "tif" )	continue;

				string msimgfilepath=filepath+"\\"+msimgname; //·��ƴ��
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