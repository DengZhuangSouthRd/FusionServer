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
#include "PrintToDB.h"

//构造函数
PrintToDB::PrintToDB(string pgconfig){
	//连接数据库
	p_pgdb = new(std::nothrow) PGDB(pgconfig);
	if (NULL == p_pgdb){
		throw runtime_error("Memory Error !");
	}
	if (p_pgdb->is_Working() == false) {
		delete p_pgdb; p_pgdb = NULL;
		throw runtime_error("PG DB Not Working ! Please Check !");
	}

	//初始化波段列表
	//GF1
	bandlist["GF1_PAN_2m"] = "b1|0.45-0.90";
	bandlist["GF1_MS_8m"] = "b1|0.45-0.52&b2|0.52-0.59&b3|0.63-0.69&b4|0.77-0.89";
	bandlist["GF1_MS_16m"] = "b1|0.45-0.52&b2|0.52-0.59&b3|0.63-0.69&b4|0.77-0.89";
	//GF2
	bandlist["GF2_PAN_1m"] = "b1|0.45-0.90";
	bandlist["GF2_MS_4m"] = "b1|0.45-0.52&b2|0.52-0.59&b3|0.63-0.69&b4|0.77-0.89";
	//WorldView2
	bandlist["WorldView2_PAN_0.5m"] = "b1|0.45-1.04";
	bandlist["WorldView2_MS_2m"] = "b1|0.45-0.51&b2|0.51-0.58&b3|0.63-0.69&b4|0.77-0.895";
	//TG1
	bandlist["TG1_PAN_0.5m"] = "b1|0.45-1.04"; //待改
	bandlist["TG1_MS_2m"] = "b1|0.45-0.51&b2|0.51-0.58&b3|0.63-0.69&b4|0.77-0.895";//待改
	bandlist["TG1_PAN_5m"] = "b1|0.5-0.8";
	bandlist["TG1_HS_20m"] = "b1|1-2.5&b2|1-2.5&b3|1-2.5&b4|1-2.5&b5|1-2.5&b6|1-2.5&b7|1-2.5&b8|1-2.5&b9|1-2.5&b10|1-2.5&\
							 b11|1-2.5&b12|1-2.5&b13|1-2.5&b14|1-2.5&b15|1-2.5&b16|1-2.5&b17|1-2.5&b18|1-2.5&b19|1-2.5&b20|1-2.5&\
							b21|1-2.5&b22|1-2.5&b23|1-2.5&b24|1-2.5&b25|1-2.5&b26|1-2.5&b27|1-2.5&b28|1-2.5&b29|1-2.5&b30|1-2.5&\
							b31|1-2.5&b32|1-2.5&b33|1-2.5&b34|1-2.5&b35|1-2.5&b36|1-2.5&b37|1-2.5&b38|1-2.5&b39|1-2.5&b40|1-2.5&\
							b41|1-2.5&b42|1-2.5&b43|1-2.5&b44|1-2.5&b45|1-2.5&b46|1-2.5&b47|1-2.5&b48|1-2.5&b49|1-2.5&b50|1-2.5&\
							b51|1-2.5&b52|1-2.5&b53|1-2.5&b54|1-2.5&b55|1-2.5&b56|1-2.5&b57|1-2.5&b58|1-2.5&b59|1-2.5&b60|1-2.5&\
							b61|1-2.5&b62|1-2.5&b63|1-2.5&b64|1-2.5&b65|1-2.5&b66|1-2.5&b67|1-2.5&b68|1-2.5&b69|1-2.5&b70|1-2.5&\
							b71|1-2.5&b72|1-2.5&b73|1-2.5&b74|1-2.5&b75|1-2.5";//待改
	//ZY3
	bandlist["ZY3_MS_6m"] = "b1|0.45-0.52&b2|0.52-0.59&b3|0.63-0.69&b4|0.77-0.89";
}

//析构函数
PrintToDB::~PrintToDB(){
	if (p_pgdb){
		delete p_pgdb; p_pgdb = NULL;
	}
}

bool PrintToDB::PrintToOptical(const string filepath, const string outpath)
{
	/*PrintToOptical		融合数据对写入t6_dataproduct_optical数据库
	地理信息以PAN为标准
	*filepath				本地文件夹路径
	文件夹
	---文件夹（命名规则如:GF1_PAN_2m_and_GF1_MS_8m）
	---文件(配对文件：GF1_PAN_2m_编号，GF1_MS_8m_编号)
	*outpath				服务器存放文件夹（程序中进行拼接，如：outpath+"/"+"GF1_PAN_2m_and_GF1_MS_8m"+"/"+"GF1_PAN_2m_1"）
	*return					true|false
	*作者：YS
	*日期：2016.07.15
	*/
	_finddata_t ImgFile;
	_finddata_t PanFile;
	long lfile;
	long lpan;
	string sql; //SQL语句

	//遍历各融合类型文件夹
	if ((lfile = _findfirst((filepath + "/*.*").c_str(), &ImgFile)) == -1)
	{
		cerr << filepath << "Not Found!" << endl;
		return false;
	}
	else{

		_findnext(lfile, &ImgFile);
		while (_findnext(lfile, &ImgFile) == 0) //获取所有文件夹
		{
			string dirname = ImgFile.name; //文件夹名字
			//
			//获取图像相关信息(通过分割文件夹名字)
			vector<string> sep = split(dirname, "_");

			cout << "Processing " << dirname << endl;
			string imgfilepath = filepath + "/" + dirname; //路径拼接

			if ((lpan = _findfirst((imgfilepath + "/*.*").c_str(), &PanFile)) == -1)
			{
				cerr << imgfilepath << "Not Found!" << endl;
				return false;
			}
			_findnext(lpan, &PanFile);
			while (_findnext(lpan, &PanFile) == 0)//获取所有文件
			{
				string panname = PanFile.name; //文件名字

				string format = panname.substr(panname.find_last_of(".") + 1);

				if (format != "tif" && format != "tiff" && format != "TIF" && format != "TIFF")	continue; //判断格式是否为tif文件
				string panpath = imgfilepath + "/" + panname; //路径拼接

				vector<double> x;
				vector<double> y;
				int panwidth, mswidth;
				int panheight, msheight;
				if (!isPanFile(panpath, x, y, panwidth, panheight)) continue;

				string tmpname = panname.substr(0, panname.find_last_of("."));
				string num = tmpname.substr(tmpname.find_last_of("_") + 1); //每对数据的序号

				string msname = sep[4] + "_" + sep[5] + "_" + sep[6] + "_" + num + ".tif";//字符拼接获取MS的文件名


				if (!isMsFile(imgfilepath + "/" + msname, mswidth, msheight)){
					cerr << "Error:" << panname << "---------" << msname << endl;
					return false;

				}
				cout << panname << "---------" << msname << endl;

				sql = "INSERT INTO wisefuse.t6_dataproduct_optical(\
					  aircraft, sensorname, productlevel, productname, bandlist, bitsperpixel,\
					  spatialresolution, width, height,\
					  ullongitude, ullatitude, lrlongitude,lrlatitude,\
					  ulxcoordinate, ulycoordinate, lrxcoordinate, lrycoordinate,\
					  timadd, producttype, datapath) VALUES ('" + sep[0] + "',"\
					  + "'" + sep[1] + "',"\
					  + "'2',"\
					  "'" + panname + "',"\
					  "'" + bandlist[sep[0] + "_" + sep[1] + "_" + sep[2]] + "',"\
					  "'16',"\
					  "'" + sep[2].substr(0, sep[2].find("m")) + "',"\
					  + "'" + to_string(panwidth) + "',"\
					  + "'" + to_string(panheight) + "',"\
					  + "'" + to_string(x[0]) + "',"\
					  + "'" + to_string(y[0]) + "',"\
					  + "'" + to_string(x[1]) + "',"\
					  + "'" + to_string(y[1]) + "',"\
					  + "'1',"\
					  + "'1',"\
					  + "'" + to_string(panwidth) + "',"\
					  + "'" + to_string(panheight) + "',"\
					  + "now(),"\
					  + "'GEOTIFF',"\
					  + "'" + outpath + "/" + dirname + "/" + "')";
				bool flag = p_pgdb->pg_exec_sql(sql); //
				if (flag == false) {
					cerr << "PG DB Execute Error." << endl;
					return false;
				}

				sql = "INSERT INTO wisefuse.t6_dataproduct_optical(\
					  aircraft, sensorname, productlevel, productname, bandlist, bitsperpixel,\
					  spatialresolution, width, height,\
					  ullongitude, ullatitude, lrlongitude,lrlatitude,\
					  ulxcoordinate, ulycoordinate, lrxcoordinate, lrycoordinate,\
					  timadd, producttype, datapath) VALUES (\
					  '" + sep[4] + "',"\
					  + "'" + sep[5] + "',"\
					  + "'2',"\
					  "'" + msname + "',"\
					  "'" + bandlist[sep[4] + "_" + sep[5] + "_" + sep[6]] + "',"\
					  "'16',"\
					  "'" + sep[6].substr(0, sep[6].find("m")) + "',"\
					  + "'" + to_string(mswidth) + "',"\
					  + "'" + to_string(msheight) + "',"\
					  + "'" + to_string(x[0]) + "',"\
					  + "'" + to_string(y[0]) + "',"\
					  + "'" + to_string(x[1]) + "',"\
					  + "'" + to_string(y[1]) + "',"\
					  + "'1',"\
					  + "'1',"\
					  + "'" + to_string(mswidth) + "',"\
					  + "'" + to_string(msheight) + "',"\
					  + "now(),"\
					  + "'GEOTIFF',"\
					  + "'" + outpath + "/" + dirname + "/" + "')";
				flag = p_pgdb->pg_exec_sql(sql); //
				if (flag == false) {
					cerr << "PG DB Execute Error." << endl;
					return false;
				}
			}
		}
	}
	return true;
}


//判断是否为pan影像
bool PrintToDB::isPanFile(const string filepath, vector<double> &x, vector<double> &y, int &width, int &height)
{
	/*isPanFile     是否Pan图像
	*filepath		图像路径
	*x				左上角, 右下角经度
	*y				左上角，右下角纬度
	*width			宽度
	*height			高度
	*return			true|false
	*作者：YS
	*日期：2016.07.15，07.17
	*/

	GDALAllRegister();         //利用GDAL读取图片，先要进行注册  
	CPLSetConfigOption("GDAL_panname_IS_UTF8", "NO");   //设置支持中文路径 
	GDALDataset *ReadDataSet = (GDALDataset*)GDALOpen(filepath.c_str(), GA_ReadOnly);
	if (NULL == ReadDataSet){
		cerr << "file：" << __FILE__ << "line：" << __LINE__ << "time：" << __DATE__ << " " << __TIME__ << endl;
		return false;
	}
	width = ReadDataSet->GetRasterXSize();
	height = ReadDataSet->GetRasterYSize();
	int bandcount = ReadDataSet->GetRasterCount();

	if (bandcount != 1)
		return false;

	double adfGeoTransform[6] = { 0, 0, 0, 0, 0, 0 };
	if (ReadDataSet->GetGeoTransform(adfGeoTransform) != CE_None){ //是否有地理信息
		cerr << filepath << " is not contain Geo Inf" << endl;
		return false;
	}

	//Proj4 
	projPJ pj_utm = NULL;
	projPJ pj_latlon = NULL;
	if (!(pj_utm = pj_init_plus("+proj=utm +zone=50 +lon_0=0 +k=1 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs"))){
		cerr << "file：" << __FILE__ << "line：" << __LINE__ << "time：" << __DATE__ << " " << __TIME__ << endl;
		GDALClose(ReadDataSet); ReadDataSet = NULL; //释放内存
		return false;
	}
	if (!(pj_latlon = pj_init_plus("+proj=longlat +datum=WGS84 +no_defs"))){
		cerr << "file：" << __FILE__ << "line：" << __LINE__ << "time：" << __DATE__ << " " << __TIME__ << endl;
		//释放内存		
		pj_free(pj_utm); pj_utm = NULL;
		GDALClose(ReadDataSet); ReadDataSet = NULL;

		return false;
	}
	//cout.precision(12);
	//左上角
	double lat = adfGeoTransform[3]; //纬度
	double lon = adfGeoTransform[0]; //经度

	pj_transform(pj_utm, pj_latlon, 1, 1, &lon, &lat, NULL);

	x.push_back(lon / DEG_TO_RAD);	//左上角经度坐标
	y.push_back(lat / DEG_TO_RAD);    //左上角纬度坐标
	//cout<<lon/DEG_TO_RAD<<"---------"<<lat/DEG_TO_RAD<<endl;

	//右下角
	lat = adfGeoTransform[3] + width * adfGeoTransform[4] + height * adfGeoTransform[5]; //纬度
	lon = adfGeoTransform[0] + width * adfGeoTransform[1] + height * adfGeoTransform[2]; //经度

	pj_transform(pj_utm, pj_latlon, 1, 1, &lon, &lat, NULL);

	x.push_back(lon / DEG_TO_RAD);	//左上角经度坐标
	y.push_back(lat / DEG_TO_RAD);    //左上角纬度坐标
	//cout<<lon/DEG_TO_RAD<<"---------"<<lat/DEG_TO_RAD<<endl;

	//释放内存
	pj_free(pj_utm); pj_utm = NULL;
	pj_free(pj_latlon); pj_latlon = NULL;
	GDALClose(ReadDataSet); ReadDataSet = NULL;
	return true;
}

//判断是否为ms影像
bool PrintToDB::isMsFile(const string filepath, int &width, int &height)
{
	/*isMsFile     是否MS图像
	*filepath		图像路径
	*width			宽度
	*height			高度
	*return			true|false
	*作者：YS
	*日期：2016.07.15，07.17
	*/
	GDALAllRegister();         //利用GDAL读取图片，先要进行注册  
	CPLSetConfigOption("GDAL_panname_IS_UTF8", "NO");   //设置支持中文路径 
	GDALDataset *ReadDataSet = (GDALDataset*)GDALOpen(filepath.c_str(), GA_ReadOnly);
	if (NULL == ReadDataSet){
		cerr << "file：" << __FILE__ << "line：" << __LINE__ << "time：" << __DATE__ << " " << __TIME__ << endl;
		return false;
	}
	width = ReadDataSet->GetRasterXSize();
	height = ReadDataSet->GetRasterYSize();
	int bandcount = ReadDataSet->GetRasterCount();

	if (bandcount == 1)
		return false;

	GDALClose(ReadDataSet); ReadDataSet = NULL;
	return true;

}

//分割字符串
vector<string> PrintToDB::split(const string str, const string separator){
		/*split     分割字符串
		*return		分割后的结果按顺序保存在vector中
		*作者：YS
		*日期：2016.07.17
		*/
		vector<string> result;
		string tmp = str;
		while (tmp.find_first_of(separator) != -1){
			result.push_back(tmp.substr(0, tmp.find_first_of(separator)));
			tmp = tmp.substr(tmp.find_first_of(separator) + 1);
		}
		result.push_back(tmp);
		return result;
	}



