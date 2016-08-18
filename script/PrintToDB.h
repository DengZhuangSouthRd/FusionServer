/********************************************************************
	created:	2016/07/15
	created:	15:7:2016   14:11
	panname: 	D:\code\vs2010\C\Fusion\PrintToDB.h
	file path:	D:\code\vs2010\C\Fusion
	file base:	PrintToDB
	file ext:	h
	author:		YS
	
	purpose:	写入库
				
*********************************************************************/
#ifndef PRINTTODB_H
#define PRINTTODB_H

#include "gdal_priv.h"  //c++ 语法
#include "ogr_spatialref.h"
#include "proj_api.h"  //proj4
#include "pgdb.h"
#include <pqxx/pqxx>
#include <string>
#include <math.h>
//#include <time.h>
#include <iostream>
#include <vector>
#include <map>
#include <io.h>
using namespace std;
using namespace pqxx;

class PrintToDB{
public:
	//构造函数
	PrintToDB(string pgconfig);
	
	//析构函数
	~PrintToDB();

public:

	//将配准好的光学图像相关信息写入数据库
	bool PrintToOptical(const string filepath, const string outpath);

public:

	//判断是否为pan影像
	bool isPanFile(const string filepath, vector<double> &x, vector<double> &y, int &width, int &height);

	//判断是否为ms影像（包含ms,hs影像）
	bool isMsFile(const string filepath, int &width, int &height);

	//分割字符串
	vector<string> split(const string str, const string separator);

private:
	map<string, string> bandlist;//存放图像波段列表
	PGDB* p_pgdb; //数据库
};

#endif