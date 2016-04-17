/********************************************************************
	created:	2016/01/20
	created:	20:1:2016   18:09
	filename: 	D:\code\vs2010\C\Fusion\CurveletFusion.cpp
	file path:	D:\code\vs2010\C\Fusion
	file base:	CurveletFusion
	file ext:	cpp
	author:		YS
	
	purpose:	CurveletFusion类实现
				Curvelet与HIS结合的融合算法
*********************************************************************/
#include "CurveletFusion.h"

#include "./curvelet/fdct_wrapping.hpp"
#include "./curvelet/fdct_wrapping_inline.hpp"

using namespace fdct_wrapping_ns;

//通过Curvelet算法实现PAN影像与MS影像融合
void CurveletFusion::Local_MaxMin_Curvelet_HIS_Fusion(const char* Input_PAN_FileName, const char* Input_MS_FileName, const char* Output_MS_FileName,int InterpolationMethod){
	/*
	 *融合方法：Curvelet与HIS
	 *Input_PAN_FileName		高分辨率图像路径及名称
	 *Input_MS_FileName			多光谱图像路径及名称（必须为三个波段）
	 *Output_MS_FileName		融合图像路径及名称
	 *InterpolationMethod       插值方法
	 *作者：YS
	 *日期：2016.01.20
	 */
	
	//待改进：（插值、变换）没有考虑节省内存，错误判断和提示
	//已改进：（1）变换考虑节省内存；
	//        （2）错误判断和提示
	//作者：YS
	//日期：2015.12.8

	//待改进：分辨率比例
	//日期：2016.2.15

	GDALAllRegister();         //利用GDAL读取图片，先要进行注册  
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");   //设置支持中文路径 

	int i,j,p,q;
	
	//读取PAN影像信息
	GdalInf* PANInf = new GdalInf();
	if (NULL == PANInf)
	{
		cerr<<"Memory Error.\n";
		cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
		exit(1);
	}

	if (PANInf->ReadImageInf(Input_PAN_FileName) !=0)
	{
		cerr<<"Read PAN Image Inf Error."<<endl;
		cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
		exit(1);
	}
	
	//读取MS影像信息
	GdalInf* MSInf = new GdalInf();
	if (NULL == MSInf)
	{
		cerr<<"Memory Error.\n";
		cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
		exit(1);
	}
	if (MSInf->ReadImageInf(Input_MS_FileName)!=0)
	{
		cerr<<"Read MS Image Inf Error."<<endl;
		cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
		exit(1);
	}
	
	
	int PAN_Width = PANInf->GetWidth(); //PAN宽度（MS影像插值后宽度）
	int PAN_Height= PANInf->GetHeight();//PAN高度（MS影像插值后高度）
	
	int MS_Width = MSInf->GetWidth();  //MS宽度
	int MS_Height = MSInf->GetHeight();//MS高度
	int MS_Bandcount = MSInf->GetBandCount(); //MS影像波段数
	
	if (MS_Bandcount < 3)
	{
		cerr<<"Curvelet Fusion Error."<<endl;
		cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
		exit(1);
	}
	else if(MS_Bandcount > 3) //如果波段数大于3 取前3个波段
	{
		MS_Bandcount = 3;
		MSInf->SetBandCount(MS_Bandcount);
	}

	MSInf->SetHeight(PAN_Height); //更新MS影像高度
	MSInf->SetWidth(PAN_Width);	  //更新MS影像宽度
	MSInf->SetGeoTransform(PANInf->GetGeoTransform());//更新地理信息

	if (MSInf->WriteImageInf(Output_MS_FileName)!=0) //创建文件，写入头文件信息
	{
		cerr<<"Write MS Image Inf Error."<<endl;
		cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
		exit(1);
	}
	
	
	/************************************************************************/
	/*						    分块处理                                    */
	/************************************************************************/
	int x=0,y=0;//初始化读取位置
	int BlockHeight,BlockWidth;//记录每次读取的大小
	ImageBlock(MS_Height,MS_Width,BlockHeight,BlockWidth);//计算每次读取的大小
	int height,width;
	int RowBlockNum = (int)ceilf(float(MS_Height)/BlockHeight); //行 块数 
	int ColBlockNum = (int)ceilf(float(MS_Width)/BlockWidth);   //列 块数
	int ratio=4; //分辨率比例
	cout<<"行块数"<<RowBlockNum<<endl;
	cout<<"列块数"<<ColBlockNum<<endl;
	cout<<"分块个数："<<RowBlockNum*ColBlockNum<<endl;
	int m,n;
	float* PANData =NULL;    //记录PAN数据
	float* MSData = NULL;    //记录MS原始数据
	float* New_MSData=NULL;  //记录MS插值后数据
	int count=1;
	for (m=1;m<=RowBlockNum;m++){
		if (m<RowBlockNum){
			height=BlockHeight;
		}
		else{
			height=MS_Height-(m-1)*BlockHeight; //边界块处理
		}
		for (n=1;n<=ColBlockNum;n++){
			if (n<ColBlockNum){
				width=BlockWidth;
			}
			else{
				width=MS_Width-(n-1)*BlockWidth;//边界块处理
			}

			cout<<"开始处理第 "<<count<<" 块"<<endl;count++;

			if (MSInf->ReadImageToBuff(Input_MS_FileName,x,y,width,height) != 0)//读取MS图像数据
			{
				cerr<<"Read MS Image Data Error."<<endl;
				cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
				exit(1);
			}

			//MS图像数据指针
			MSData = MSInf->GetImgData();	  
			New_MSData = new float[height*ratio*width*ratio*MS_Bandcount];  //保存MS插值后的影像
			if (NULL == New_MSData)
			{
				cerr<<"Memory Error.\n";
				cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
				exit(1);
			}
			//插值
			//插值方法：Nearest/Linear/CubicConv
			Interpolation(MSData, height,width ,MS_Bandcount, New_MSData, height*ratio, width*ratio, InterpolationMethod);
			
			//释放内存
			MSInf->ClearImageData(); MSData = NULL;
			
			//读取PAN图像数据
			if (PANInf->ReadImageToBuff(Input_PAN_FileName,x*ratio,y*ratio,width*ratio,height*ratio) != 0) 
			{
			
				cerr<<"Read PAN Image Data Error."<<endl;
				cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
				exit(1);
			}

			PANData = PANInf->GetImgData();    //PAN图像数据指针


			int ROWS = MS_Bandcount;
			int COLS = height*ratio*width*ratio;
	
			//对MS做HSI变换
			RGB2HSI(New_MSData, height*ratio, width*ratio, MS_Bandcount);

			//对PAN影像灰度拉伸

			//for (i = 0; i < COLS; i++)
			//	I[i] = DATA2D(New_MSData, 2, i, COLS);

			float max,min;

			MaxMin(PANData,height*ratio,width*ratio,max,min);
			//cout<<"最大值："<<max<<" 最小值："<<min<<endl;

			for (i = 0; i < COLS; i++)
				PANData[i] = (PANData[i]-min)/(max-min);

			MaxMin(New_MSData+2*COLS,height*ratio,width*ratio,max,min); 
			//cout<<"最大值："<<max<<" 最小值："<<min<<endl;

			for (i = 0; i < COLS; i++)
				PANData[i] = PANData[i]*(max-min)+min;

			//MaxMin(PANData,height*ratio,width*ratio,max,min);
			//cout<<"最大值："<<max<<" 最小值："<<min<<endl;

			//进行PAN影像与MS影像融合
			//  nbscales -- the total number of scales for subband decomposition
			int nbscales=3;
			//  nbangles_coarse -- the number of angles in the 2nd coarest scale
			int nbangles_coarse=4;
			//  ac -- ac==1 use curvelet at the finest level, ac==0, use wavelet at the finest level 
			int ac=1;
			CpxNumMat PanMat(height*ratio,width*ratio);
			for(i=0; i<height*ratio; i++)
				for(j=0; j<width*ratio; j++){
					PanMat(i,j)= cpx(DATA2D(PANData, i, j, width*ratio),0);
					//IMat(i,j) = cpx(DATA2D(I->imgdata, i, j, PANData->width),0);
				}
			//释放内存
	
			PANInf->ClearImageData();PANData=NULL;

			vector< vector<CpxNumMat> > CPanMat;
			fdct_wrapping(height*ratio,width*ratio, nbscales, nbangles_coarse, ac, PanMat, CPanMat);
			//释放内存
			//PanMat.~NumMat();
			delete[] PanMat._data;PanMat._data=NULL;//clear(PanMat);

			CpxNumMat IMat(height*ratio,width*ratio);
			for(i=0; i<height*ratio; i++)
				for(j=0; j<width*ratio; j++){
					//PanMat(i,j)= cpx(DATA2D(PANData->imgdata, i, j, PANData->width),0);
					IMat(i,j) = cpx(New_MSData[2*COLS+i*width*ratio+j],0);
				}

			vector< vector<CpxNumMat> > CIMat;
			fdct_wrapping(height*ratio,width*ratio, nbscales, nbangles_coarse, ac, IMat, CIMat);

			//释放内存
			//IMat.~NumMat();
			delete[] IMat._data;IMat._data=NULL;//clear(IMat);

			for (i=0;i<CPanMat[0][0].m();i++)
				for (j=0;j<CPanMat[0][0].n();j++){
					CPanMat[0][0](i,j)=0.5*CPanMat[0][0](i,j)+0.5*CIMat[0][0](i,j);
				}
			for (i=1;i<CPanMat.size();i++)
				for (j=0;j<CPanMat[i].size();j++)
					for (p=0;p<CPanMat[i][j].m();p++)
						for (q=0;q<CPanMat[i][j].n();q++){
							CPanMat[i][j](p,q)=	abs(CPanMat[i][j](p,q)) > abs(CIMat[i][j](p,q))? CPanMat[i][j](p,q):CIMat[i][j](p,q);
						}
			//释放内存
			CIMat.clear();

			CpxNumMat Fusion(height*ratio,width*ratio); 
			ifdct_wrapping(height*ratio,width*ratio, nbscales, nbangles_coarse, ac, CPanMat, Fusion);

			//释放内存
			CPanMat.clear();
			//用融合后的图像替换I分量生成新HSI
			for(i=0; i<height*ratio; i++)
				for(j=0; j<width*ratio; j++){
					//if (Fusion(i,j).imag()!=0)
					//{
					//	cout<<"image"<<endl;
					//}
					DATA2D(New_MSData, 2, i*width*ratio+j, COLS) = Fusion(i,j).real() > 0 ? (float)Fusion(i,j).real():0;// 
				}
			//释放内存
			delete[] Fusion._data;Fusion._data=NULL;//clear(Fusion);	

			//将MS从HSI变换到RGB空间
			HSI2RGB(New_MSData, height*ratio,width*ratio, MS_Bandcount);


			//GDAL写文件
			MSInf->SetImgData(New_MSData);
			if(MSInf->WriteImageFromBuff(Output_MS_FileName,x*ratio,y*ratio,width*ratio,height*ratio)!=0){
				cerr<<"Write MS Image Data Error."<<endl;
				cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
				exit(1);
			}
			//释放内存
			MSInf->ClearImageData();New_MSData = NULL;
			    	
			x=n*width;	
		}
		y=m*height;
		x=0;
	}
/************************************************************************/
/*						分块处理结束                                    */
/************************************************************************/
	
	//释放内存
	delete MSInf;	MSInf = NULL;
	delete PANInf;	PANInf = NULL;


}

void CurveletFusion::Local_MeanStd_Curvelet_HIS_Fusion(const char* Input_PAN_FileName, const char* Input_MS_FileName, const char* Output_MS_FileName,int InterpolationMethod){
	/*
	 *融合方法：Curvelet与HIS
	 *Input_PAN_FileName		高分辨率图像路径及名称
	 *Input_MS_FileName			多光谱图像路径及名称（必须为三个波段）
	 *Output_MS_FileName		融合图像路径及名称
	 *InterpolationMethod       插值方法
	 *作者：YS
	 *日期：2016.01.20
	 */
	
	//待改进：（插值、变换）没有考虑节省内存，错误判断和提示
	//已改进：（1）变换考虑节省内存；
	//        （2）错误判断和提示
	//作者：YS
	//日期：2015.12.8

	GDALAllRegister();         //利用GDAL读取图片，先要进行注册  
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");   //设置支持中文路径 

	int i,j,p,q;
	
	//读取PAN影像信息
	GdalInf* PANInf = new GdalInf();
	if (NULL == PANInf)
	{
		cerr<<"Memory Error.\n";
		cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
		exit(1);
	}

	if (PANInf->ReadImageInf(Input_PAN_FileName) !=0)
	{
		cerr<<"Read PAN Image Inf Error."<<endl;
		cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
		exit(1);
	}
	
	//读取MS影像信息
	GdalInf* MSInf = new GdalInf();
	if (NULL == MSInf)
	{
		cerr<<"Memory Error.\n";
		cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
		exit(1);
	}
	if (MSInf->ReadImageInf(Input_MS_FileName)!=0)
	{
		cerr<<"Read MS Image Inf Error."<<endl;
		cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
		exit(1);
	}
	
	
	int PAN_Width = PANInf->GetWidth(); //PAN宽度（MS影像插值后宽度）
	int PAN_Height= PANInf->GetHeight();//PAN高度（MS影像插值后高度）
	
	int MS_Width = MSInf->GetWidth();  //MS宽度
	int MS_Height = MSInf->GetHeight();//MS高度
	int MS_Bandcount = MSInf->GetBandCount(); //MS影像波段数
	
	if (MS_Bandcount < 3)
	{
		cerr<<"Curvelet Fusion Error."<<endl;
		cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
		exit(1);
	}
	else if(MS_Bandcount > 3) //如果波段数大于3 取前3个波段
	{
		MS_Bandcount = 3;
		MSInf->SetBandCount(MS_Bandcount);
	}

	MSInf->SetHeight(PAN_Height); //更新MS影像高度
	MSInf->SetWidth(PAN_Width);	  //更新MS影像宽度
	MSInf->SetGeoTransform(PANInf->GetGeoTransform());//更新地理信息

	if (MSInf->WriteImageInf(Output_MS_FileName)!=0) //创建文件，写入头文件信息
	{
		cerr<<"Write MS Image Inf Error."<<endl;
		cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
		exit(1);
	}
	
	
	/************************************************************************/
	/*						    分块处理                                    */
	/************************************************************************/
	int x=0,y=0;//初始化读取位置
	int BlockHeight,BlockWidth;//记录每次读取的大小
	ImageBlock(MS_Height,MS_Width,BlockHeight,BlockWidth);//计算每次读取的大小
	int height,width;
	int RowBlockNum = (int)ceilf(float(MS_Height)/BlockHeight); //行 块数 
	int ColBlockNum = (int)ceilf(float(MS_Width)/BlockWidth);   //列 块数
	int ratio=4; //分辨率比例
	cout<<"行块数"<<RowBlockNum<<endl;
	cout<<"列块数"<<ColBlockNum<<endl;
	cout<<"分块个数："<<RowBlockNum*ColBlockNum<<endl;
	int m,n;
	float* PANData =NULL;    //记录PAN数据
	float* MSData = NULL;    //记录MS原始数据
	float* New_MSData=NULL;  //记录MS插值后数据
	int count=1;
	for (m=1;m<=RowBlockNum;m++){
		if (m<RowBlockNum){
			height=BlockHeight;
		}
		else{
			height=MS_Height-(m-1)*BlockHeight; //边界块处理
		}
		for (n=1;n<=ColBlockNum;n++){
			if (n<ColBlockNum){
				width=BlockWidth;
			}
			else{
				width=MS_Width-(n-1)*BlockWidth;//边界块处理
			}

			cout<<"开始处理第 "<<count<<" 块"<<endl;count++;

			if (MSInf->ReadImageToBuff(Input_MS_FileName,x,y,width,height) != 0)//读取MS图像数据
			{
				cerr<<"Read MS Image Data Error."<<endl;
				cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
				exit(1);
			}

			//MS图像数据指针
			MSData = MSInf->GetImgData();	  
			New_MSData = new float[height*ratio*width*ratio*MS_Bandcount];  //保存MS插值后的影像
			if (NULL == New_MSData)
			{
				cerr<<"Memory Error.\n";
				cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
				exit(1);
			}
			//插值
			//插值方法：Nearest/Linear/CubicConv
			Interpolation(MSData, height,width ,MS_Bandcount, New_MSData, height*ratio, width*ratio, InterpolationMethod);
			
			//释放内存
			MSInf->ClearImageData(); MSData = NULL;
			
			//读取PAN图像数据
			if (PANInf->ReadImageToBuff(Input_PAN_FileName,x*ratio,y*ratio,width*ratio,height*ratio) != 0) 
			{
			
				cerr<<"Read PAN Image Data Error."<<endl;
				cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
				exit(1);
			}

			PANData = PANInf->GetImgData();    //PAN图像数据指针


			int ROWS = MS_Bandcount;
			int COLS = height*ratio*width*ratio;
	
			//对MS做HSI变换
			RGB2HSI(New_MSData, height*ratio, width*ratio, MS_Bandcount);

			//对PAN影像灰度拉伸

			//for (i = 0; i < COLS; i++)
			//	I[i] = DATA2D(New_MSData, 2, i, COLS);

			float mean,s;//均值 标准差

			MeanStd(PANData,height*ratio,width*ratio,s,mean);

			for (i = 0; i < COLS; i++)
				PANData[i] = (PANData[i]-mean)/(s);

			MeanStd(New_MSData+2*COLS,height*ratio,width*ratio,s,mean); 		

			for (i = 0; i < COLS; i++)
				PANData[i] = PANData[i]*(s)+mean;


			//进行PAN影像与MS影像融合
			//  nbscales -- the total number of scales for subband decomposition
			int nbscales=3;
			//  nbangles_coarse -- the number of angles in the 2nd coarest scale
			int nbangles_coarse=4;
			//  ac -- ac==1 use curvelet at the finest level, ac==0, use wavelet at the finest level 
			int ac=1;
			CpxNumMat PanMat(height*ratio,width*ratio);
			for(i=0; i<height*ratio; i++)
				for(j=0; j<width*ratio; j++){
					PanMat(i,j)= cpx(DATA2D(PANData, i, j, width*ratio),0);
					//IMat(i,j) = cpx(DATA2D(I->imgdata, i, j, PANData->width),0);
				}
			//释放内存
	
			PANInf->ClearImageData();PANData=NULL;

			vector< vector<CpxNumMat> > CPanMat;
			fdct_wrapping(height*ratio,width*ratio, nbscales, nbangles_coarse, ac, PanMat, CPanMat);
			//释放内存
			//PanMat.~NumMat();
			delete[] PanMat._data;PanMat._data=NULL;//clear(PanMat);

			CpxNumMat IMat(height*ratio,width*ratio);
			for(i=0; i<height*ratio; i++)
				for(j=0; j<width*ratio; j++){
					//PanMat(i,j)= cpx(DATA2D(PANData->imgdata, i, j, PANData->width),0);
					IMat(i,j) = cpx(New_MSData[2*COLS+i*width*ratio+j],0);
				}

			vector< vector<CpxNumMat> > CIMat;
			fdct_wrapping(height*ratio,width*ratio, nbscales, nbangles_coarse, ac, IMat, CIMat);

			//释放内存
			//IMat.~NumMat();
			delete[] IMat._data;IMat._data=NULL;//clear(IMat);

			for (i=0;i<CPanMat[0][0].m();i++)
				for (j=0;j<CPanMat[0][0].n();j++){
					CPanMat[0][0](i,j)=0.5*CPanMat[0][0](i,j)+0.5*CIMat[0][0](i,j);
				}
			for (i=1;i<CPanMat.size();i++)
				for (j=0;j<CPanMat[i].size();j++)
					for (p=0;p<CPanMat[i][j].m();p++)
						for (q=0;q<CPanMat[i][j].n();q++){
							CPanMat[i][j](p,q)=	abs(CPanMat[i][j](p,q)) > abs(CIMat[i][j](p,q))? CPanMat[i][j](p,q):CIMat[i][j](p,q);
						}
			//释放内存
			CIMat.clear();

			CpxNumMat Fusion(height*ratio,width*ratio); 
			ifdct_wrapping(height*ratio,width*ratio, nbscales, nbangles_coarse, ac, CPanMat, Fusion);

			//释放内存
			CPanMat.clear();
			//用融合后的图像替换I分量生成新HSI
			for(i=0; i<height*ratio; i++)
				for(j=0; j<width*ratio; j++){

					DATA2D(New_MSData, 2, i*width*ratio+j, COLS) = Fusion(i,j).real() > 0 ? (float)Fusion(i,j).real():0;// 
				}
			//释放内存
			delete[] Fusion._data;Fusion._data=NULL;//clear(Fusion);	

			//将MS从HSI变换到RGB空间
			HSI2RGB(New_MSData, height*ratio,width*ratio, MS_Bandcount);


			//GDAL写文件
			MSInf->SetImgData(New_MSData);
			if(MSInf->WriteImageFromBuff(Output_MS_FileName,x*ratio,y*ratio,width*ratio,height*ratio)!=0){
				cerr<<"Write MS Image Data Error."<<endl;
				cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
				exit(1);
			}
			//释放内存
			MSInf->ClearImageData();New_MSData = NULL;
			    	
			x=n*width;	
		}
		y=m*height;
		x=0;
	}
/************************************************************************/
/*						分块处理结束                                    */
/************************************************************************/
	
	//释放内存
	delete MSInf;	MSInf = NULL;
	delete PANInf;	PANInf = NULL;


}

void CurveletFusion::Global_MaxMin_Curvelet_HIS_Fusion(const char* Input_PAN_FileName, const char* Input_MS_FileName, const char* Output_MS_FileName,int InterpolationMethod){
	/*
	 *融合方法：Curvelet与HIS
	 *Input_PAN_FileName		高分辨率图像路径及名称
	 *Input_MS_FileName			多光谱图像路径及名称（必须为三个波段）
	 *Output_MS_FileName		融合图像路径及名称
	 *InterpolationMethod       插值方法
	 *作者：YS
	 *日期：2016.01.20
	 */

	GDALAllRegister();         //利用GDAL读取图片，先要进行注册  
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");   //设置支持中文路径 

	int i,j,p,q;

	//读取PAN影像信息
	GdalInf* PANInf = new GdalInf();
	if (NULL == PANInf)
	{
		cerr<<"Memory Error.\n";
		cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
		exit(1);
	}

	if (PANInf->ReadImageInf(Input_PAN_FileName) !=0)
	{
		cerr<<"Read PAN Image Inf Error."<<endl;
		cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
		exit(1);
	}

	//读取MS影像信息
	GdalInf* MSInf = new GdalInf();
	if (NULL == MSInf)
	{
		cerr<<"Memory Error.\n";
		cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
		exit(1);
	}
	if (MSInf->ReadImageInf(Input_MS_FileName)!=0)
	{
		cerr<<"Read MS Image Inf Error."<<endl;
		cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
		exit(1);
	}


	int PAN_Width = PANInf->GetWidth(); //PAN宽度（MS影像插值后宽度）
	int PAN_Height= PANInf->GetHeight();//PAN高度（MS影像插值后高度）

	int MS_Width = MSInf->GetWidth();  //MS宽度
	int MS_Height = MSInf->GetHeight();//MS高度
	int MS_Bandcount = MSInf->GetBandCount(); //MS影像波段数

	if (MS_Bandcount < 3)
	{
		cerr<<"HSI Fusion Error."<<endl;
		cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
		exit(1);
	}
	else if(MS_Bandcount > 3) //如果波段数大于3 取前3个波段
	{
		MS_Bandcount = 3;
		MSInf->SetBandCount(MS_Bandcount);
	}

	MSInf->SetHeight(PAN_Height); //更新MS影像高度
	MSInf->SetWidth(PAN_Width);	  //更新MS影像宽度
	MSInf->SetGeoTransform(PANInf->GetGeoTransform());//更新地理信息

	if (MSInf->WriteImageInf(Output_MS_FileName)!=0) //创建文件，写入头文件信息
	{
		cerr<<"Write MS Image Inf Error."<<endl;
		cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
		exit(1);
	}

	/************************************************************************/
	/*						    分块处理                                    */
	/************************************************************************/
	int x=0,y=0;//初始化读取位置
	int BlockHeight,BlockWidth;//记录每次读取的大小
	ImageBlock(MS_Height,MS_Width,BlockHeight,BlockWidth);//计算每次读取的大小
	int height,width;
	int RowBlockNum = (int)ceilf(float(MS_Height)/BlockHeight); //行 块数 
	int ColBlockNum = (int)ceilf(float(MS_Width)/BlockWidth);   //列 块数
	int ratio=4; //分辨率比例
	cout<<"行块数"<<RowBlockNum<<endl;
	cout<<"列块数"<<ColBlockNum<<endl;
	cout<<"分块个数："<<RowBlockNum*ColBlockNum<<endl;
	int m,n;
	float* PANData =NULL;    //记录PAN数据
	float* MSData = NULL;    //记录MS原始数据
	float* New_MSData = NULL;  //记录MS插值后数据
	float max,min;           //局部极值
	float panmax,panmin,imax,imin;//全局极值
	panmax = imax = -1;
	panmin = imin =100000;
	for (m=1;m<=RowBlockNum;m++){
		if (m<RowBlockNum){
			height=BlockHeight;
		}
		else{
			height=MS_Height-(m-1)*BlockHeight; //边界块处理
		}
		for (n=1;n<=ColBlockNum;n++){
			if (n<ColBlockNum){
				width=BlockWidth;
			}
			else{
				width=MS_Width-(n-1)*BlockWidth;//边界块处理
			}

			//读取MS图像数据
			if (MSInf->ReadImageToBuff(Input_MS_FileName,x,y,width,height) != 0)
			{
				cerr<<"Read MS Image Data Error.\n";
				cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
				exit(1);
			}

			MSData = MSInf->GetImgData();	  //MS图像数据指针
			New_MSData = new float[height*ratio*width*ratio*MS_Bandcount];  //保存MS插值后的影像

			if (NULL == New_MSData)//
			{
				cerr<<"Memory Error.\n";
				cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
				exit(1);
			}
			//插值
			//插值方法：Nearest/Linear/CubicConv
			Interpolation(MSData, height,width ,MS_Bandcount, New_MSData, height*ratio, width*ratio, InterpolationMethod);

			//释放内存
			MSInf->ClearImageData();MSData = NULL;  

			int ROWS = MS_Bandcount;
			int COLS = height*ratio*width*ratio;

			//求MS的I分量
			for (i = 0; i < COLS; i++)
			{
				DATA2D(New_MSData, 2, i, COLS)=(DATA2D(New_MSData, 0, i, COLS)\
					+DATA2D(New_MSData, 1, i, COLS)\
					+DATA2D(New_MSData, 2, i, COLS))/3;

			}

			//求极值
			MaxMin(New_MSData+2*COLS,height*ratio,width*ratio,max,min); 
			if (imax < max) imax = max;
			if (imin > min) imin = min;			

			//释放内存
			delete[] New_MSData;New_MSData = NULL;

			//读取PAN图像数据
			if (PANInf->ReadImageToBuff(Input_PAN_FileName,x*ratio,y*ratio,width*ratio,height*ratio) != 0) 
			{
				cerr<<"Read PAN Image Data Error.\n";
				cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
				exit(1);
			}
			PANData = PANInf->GetImgData();    //PAN图像数据指针

			//求极值
			MaxMin(PANData,height*ratio,width*ratio,max,min);
			if (panmax < max) panmax = max;
			if (panmin > min) panmin = min;

			//释放内存
			PANInf->ClearImageData();PANData = NULL; 

			x=n*width;	
		}
		y=m*height;
		x=0;
	}
	cout<<imax <<" "<<imin<<endl;
	cout<<panmax<<" "<<panmin<<endl;
	int count=1;
	x=0;
	y=0;
	for (m=1;m<=RowBlockNum;m++){
		if (m<RowBlockNum){
			height=BlockHeight;
		}
		else{
			height=MS_Height-(m-1)*BlockHeight; //边界块处理
		}
		for (n=1;n<=ColBlockNum;n++){
			if (n<ColBlockNum){
				width=BlockWidth;
			}
			else{
				width=MS_Width-(n-1)*BlockWidth;//边界块处理
			}
			cout<<"开始处理第 "<<count<<" 块"<<endl;count++;

			if (MSInf->ReadImageToBuff(Input_MS_FileName,x,y,width,height) != 0)//读取MS图像数据
			{
				cerr<<"Read MS Image Data Error."<<endl;
				cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
				exit(1);
			}

			MSData = MSInf->GetImgData();	  //MS图像数据指针
			New_MSData = new float[height*ratio*width*ratio*MS_Bandcount];  //保存MS插值后的影像

			if (NULL == New_MSData)//
			{
				cerr<<"Memory Error.\n";
				cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
				exit(1);
			}
			//插值
			//插值方法：Nearest/Linear/CubicConv
			Interpolation(MSData, height,width ,MS_Bandcount, New_MSData, height*ratio, width*ratio, InterpolationMethod);

			//释放内存
			MSInf->ClearImageData(); MSData = NULL;

			if (PANInf->ReadImageToBuff(Input_PAN_FileName,x*ratio,y*ratio,width*ratio,height*ratio) != 0) //读取PAN图像数据
			{
				cerr<<"Read PAN Image Data Error."<<endl;
				cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
				exit(1);
			}
			PANData = PANInf->GetImgData();    //PAN图像数据指针

			int ROWS = MS_Bandcount;
			int COLS = height*ratio*width*ratio;


			//对MS做HSI变换
			RGB2HSI(New_MSData, height*ratio, width*ratio, MS_Bandcount);

			//对PAN影像灰度拉伸

			for (i = 0; i < COLS; i++)
				PANData[i] = (PANData[i]-panmin)/(panmax-panmin);

			for (i = 0; i < COLS; i++)
				PANData[i] = PANData[i]*(imax-imin)+imin;

			//进行PAN影像与MS影像融合
			//  nbscales -- the total number of scales for subband decomposition
			int nbscales=3;
			//  nbangles_coarse -- the number of angles in the 2nd coarest scale
			int nbangles_coarse=4;
			//  ac -- ac==1 use curvelet at the finest level, ac==0, use wavelet at the finest level 
			int ac=1;
			CpxNumMat PanMat(height*ratio,width*ratio);
			for(i=0; i<height*ratio; i++)
				for(j=0; j<width*ratio; j++){
					PanMat(i,j)= cpx(DATA2D(PANData, i, j, width*ratio),0);
					//IMat(i,j) = cpx(DATA2D(I->imgdata, i, j, PANData->width),0);
				}
				//释放内存

			PANInf->ClearImageData();PANData=NULL;

			vector< vector<CpxNumMat> > CPanMat;
			fdct_wrapping(height*ratio,width*ratio, nbscales, nbangles_coarse, ac, PanMat, CPanMat);
			//释放内存
			//PanMat.~NumMat();
			delete[] PanMat._data;PanMat._data=NULL;//clear(PanMat);

			CpxNumMat IMat(height*ratio,width*ratio);
			for(i=0; i<height*ratio; i++)
				for(j=0; j<width*ratio; j++){
					//PanMat(i,j)= cpx(DATA2D(PANData->imgdata, i, j, PANData->width),0);
					IMat(i,j) = cpx(New_MSData[2*COLS+i*width*ratio+j],0);
				}

			vector< vector<CpxNumMat> > CIMat;
			fdct_wrapping(height*ratio,width*ratio, nbscales, nbangles_coarse, ac, IMat, CIMat);

			//释放内存
			//IMat.~NumMat();
			delete[] IMat._data;IMat._data=NULL;//clear(IMat);

			for (i=0;i<CPanMat[0][0].m();i++)
				for (j=0;j<CPanMat[0][0].n();j++){
					CPanMat[0][0](i,j)=0.5*CPanMat[0][0](i,j)+0.5*CIMat[0][0](i,j);
				}
			for (i=1;i<CPanMat.size();i++)
				for (j=0;j<CPanMat[i].size();j++)
					for (p=0;p<CPanMat[i][j].m();p++)
						for (q=0;q<CPanMat[i][j].n();q++){
							CPanMat[i][j](p,q)=	abs(CPanMat[i][j](p,q)) > abs(CIMat[i][j](p,q))? CPanMat[i][j](p,q):CIMat[i][j](p,q);
						}
			//释放内存
			CIMat.clear();

			CpxNumMat Fusion(height*ratio,width*ratio); 
			ifdct_wrapping(height*ratio,width*ratio, nbscales, nbangles_coarse, ac, CPanMat, Fusion);

			//释放内存
			CPanMat.clear();
			//用融合后的图像替换I分量生成新HSI
			for(i=0; i<height*ratio; i++)
				for(j=0; j<width*ratio; j++){
					//if (Fusion(i,j).imag()!=0)
					//{
					//	cout<<"image"<<endl;
					//}
					DATA2D(New_MSData, 2, i*width*ratio+j, COLS) = Fusion(i,j).real() > 0 ? (float)Fusion(i,j).real():0;// 
				}
			//释放内存
			delete[] Fusion._data;Fusion._data=NULL;//clear(Fusion); 

			//将MS从HSI变换到RGB空间
			HSI2RGB(New_MSData, height*ratio,width*ratio, MS_Bandcount);

			//GDAL写文件
			MSInf->SetImgData(New_MSData);
			if(MSInf->WriteImageFromBuff(Output_MS_FileName,x*ratio,y*ratio,width*ratio,height*ratio)!=0){
				cerr<<"Write MS Image Data Error."<<endl;
				cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
				exit(1);
			}
			//释放内存
			MSInf->ClearImageData();New_MSData = NULL;

			x=n*width;	
		}
		y=m*height;
		x=0;
	}
	/************************************************************************/
	/*						分块处理结束                                    */
	/************************************************************************/

	//释放内存
	delete MSInf;	MSInf = NULL;
	delete PANInf;	PANInf = NULL;

}
void CurveletFusion::Global_MeanStd_Curvelet_HIS_Fusion(const char* Input_PAN_FileName, const char* Input_MS_FileName, const char* Output_MS_FileName,int InterpolationMethod){
	/*
	 *融合方法：Curvelet与HIS
	 *Input_PAN_FileName		高分辨率图像路径及名称
	 *Input_MS_FileName			多光谱图像路径及名称（必须为三个波段）
	 *Output_MS_FileName		融合图像路径及名称
	 *InterpolationMethod       插值方法
	 *作者：YS
	 *日期：2016.01.20
	 */

	GDALAllRegister();         //利用GDAL读取图片，先要进行注册  
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");   //设置支持中文路径 

	int i,j,p,q;

	//读取PAN影像信息
	GdalInf* PANInf = new GdalInf();
	if (NULL == PANInf)
	{
		cerr<<"Memory Error.\n";
		cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
		exit(1);
	}

	if (PANInf->ReadImageInf(Input_PAN_FileName) !=0)
	{
		cerr<<"Read PAN Image Inf Error."<<endl;
		cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
		exit(1);
	}

	//读取MS影像信息
	GdalInf* MSInf = new GdalInf();
	if (NULL == MSInf)
	{
		cerr<<"Memory Error.\n";
		cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
		exit(1);
	}
	if (MSInf->ReadImageInf(Input_MS_FileName)!=0)
	{
		cerr<<"Read MS Image Inf Error."<<endl;
		cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
		exit(1);
	}


	int PAN_Width = PANInf->GetWidth(); //PAN宽度（MS影像插值后宽度）
	int PAN_Height= PANInf->GetHeight();//PAN高度（MS影像插值后高度）

	int MS_Width = MSInf->GetWidth();  //MS宽度
	int MS_Height = MSInf->GetHeight();//MS高度
	int MS_Bandcount = MSInf->GetBandCount(); //MS影像波段数

	if (MS_Bandcount < 3)
	{
		cerr<<"HSI Fusion Error."<<endl;
		cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
		exit(1);
	}
	else if(MS_Bandcount > 3) //如果波段数大于3 取前3个波段
	{
		MS_Bandcount = 3;
		MSInf->SetBandCount(MS_Bandcount);
	}

	MSInf->SetHeight(PAN_Height); //更新MS影像高度
	MSInf->SetWidth(PAN_Width);	  //更新MS影像宽度
	MSInf->SetGeoTransform(PANInf->GetGeoTransform());//更新地理信息

	if (MSInf->WriteImageInf(Output_MS_FileName)!=0) //创建文件，写入头文件信息
	{
		cerr<<"Write MS Image Inf Error."<<endl;
		cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
		exit(1);
	}

	/************************************************************************/
	/*						    分块处理                                    */
	/************************************************************************/
	int x=0,y=0;//初始化读取位置
	int BlockHeight,BlockWidth;//记录每次读取的大小
	ImageBlock(MS_Height,MS_Width,BlockHeight,BlockWidth);//计算每次读取的大小
	int height,width;
	int RowBlockNum = (int)ceilf(float(MS_Height)/BlockHeight); //行 块数 
	int ColBlockNum = (int)ceilf(float(MS_Width)/BlockWidth);   //列 块数
	int ratio=4; //分辨率比例
	cout<<"行块数"<<RowBlockNum<<endl;
	cout<<"列块数"<<ColBlockNum<<endl;
	cout<<"分块个数："<<RowBlockNum*ColBlockNum<<endl;
	int m,n;
	float* PANData =NULL;    //记录PAN数据
	float* MSData = NULL;    //记录MS原始数据
	float* New_MSData = NULL;  //记录MS插值后数据
	//float mean,s;//均值 标准差           //局部极值
	float panmean,panvar,imean,ivar;//全局极值
	panmean = imean = panvar = ivar = 0;

	//求均值
	for (m=1;m<=RowBlockNum;m++){
		if (m<RowBlockNum){
			height=BlockHeight;
		}
		else{
			height=MS_Height-(m-1)*BlockHeight; //边界块处理
		}
		for (n=1;n<=ColBlockNum;n++){
			if (n<ColBlockNum){
				width=BlockWidth;
			}
			else{
				width=MS_Width-(n-1)*BlockWidth;//边界块处理
			}

			//读取MS图像数据
			if (MSInf->ReadImageToBuff(Input_MS_FileName,x,y,width,height) != 0)
			{
				cerr<<"Read MS Image Data Error.\n";
				cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
				exit(1);
			}

			MSData = MSInf->GetImgData();	  //MS图像数据指针
			New_MSData = new float[height*ratio*width*ratio*MS_Bandcount];  //保存MS插值后的影像

			if (NULL == New_MSData)//
			{
				cerr<<"Memory Error.\n";
				cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
				exit(1);
			}
			//插值
			//插值方法：Nearest/Linear/CubicConv
			Interpolation(MSData, height,width ,MS_Bandcount, New_MSData, height*ratio, width*ratio, InterpolationMethod);

			//释放内存
			MSInf->ClearImageData();MSData = NULL;  

			int ROWS = MS_Bandcount;
			int COLS = height*ratio*width*ratio;

			//求MS的I分量
			for (i = 0; i < COLS; i++)
			{
				DATA2D(New_MSData, 2, i, COLS)=(DATA2D(New_MSData, 0, i, COLS)\
					+DATA2D(New_MSData, 1, i, COLS)\
					+DATA2D(New_MSData, 2, i, COLS))/3;
				imean += DATA2D(New_MSData, 2, i, COLS);
			}


			//释放内存
			delete[] New_MSData;New_MSData = NULL;

			//读取PAN图像数据
			if (PANInf->ReadImageToBuff(Input_PAN_FileName,x*ratio,y*ratio,width*ratio,height*ratio) != 0) 
			{
				cerr<<"Read PAN Image Data Error.\n";
				cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
				exit(1);
			}
			PANData = PANInf->GetImgData();    //PAN图像数据指针

			for (i = 0; i < COLS; i++)
			{
				panmean += PANData[i];
			}


			//释放内存
			PANInf->ClearImageData();PANData = NULL; 

			x=n*width;	
		}
		y=m*height;
		x=0;
	}
	panmean = panmean/(PAN_Height*PAN_Width);
	imean = imean/(PAN_Height*PAN_Width);

	//求方差
	x=0;y=0;
	for (m=1;m<=RowBlockNum;m++){
		if (m<RowBlockNum){
			height=BlockHeight;
		}
		else{
			height=MS_Height-(m-1)*BlockHeight; //边界块处理
		}
		for (n=1;n<=ColBlockNum;n++){
			if (n<ColBlockNum){
				width=BlockWidth;
			}
			else{
				width=MS_Width-(n-1)*BlockWidth;//边界块处理
			}

			//读取MS图像数据
			if (MSInf->ReadImageToBuff(Input_MS_FileName,x,y,width,height) != 0)
			{
				cerr<<"Read MS Image Data Error.\n";
				cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
				exit(1);
			}

			MSData = MSInf->GetImgData();	  //MS图像数据指针
			New_MSData = new float[height*ratio*width*ratio*MS_Bandcount];  //保存MS插值后的影像

			if (NULL == New_MSData)//
			{
				cerr<<"Memory Error.\n";
				cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
				exit(1);
			}
			//插值
			//插值方法：Nearest/Linear/CubicConv
			Interpolation(MSData, height,width ,MS_Bandcount, New_MSData, height*ratio, width*ratio, InterpolationMethod);

			//释放内存
			MSInf->ClearImageData();MSData = NULL;  

			int ROWS = MS_Bandcount;
			int COLS = height*ratio*width*ratio;

			//求MS的I分量
			for (i = 0; i < COLS; i++)
			{
				DATA2D(New_MSData, 2, i, COLS)=(DATA2D(New_MSData, 0, i, COLS)\
					+DATA2D(New_MSData, 1, i, COLS)\
					+DATA2D(New_MSData, 2, i, COLS))/3;
				ivar += (DATA2D(New_MSData, 2, i, COLS)-imean)*(DATA2D(New_MSData, 2, i, COLS)-imean);
			}


			//释放内存
			delete[] New_MSData;New_MSData = NULL;

			//读取PAN图像数据
			if (PANInf->ReadImageToBuff(Input_PAN_FileName,x*ratio,y*ratio,width*ratio,height*ratio) != 0) 
			{
				cerr<<"Read PAN Image Data Error.\n";
				cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
				exit(1);
			}
			PANData = PANInf->GetImgData();    //PAN图像数据指针

			for (i = 0; i < COLS; i++)
			{
				panvar += (PANData[i]-panmean)*(PANData[i]-panmean);
			}


			//释放内存
			PANInf->ClearImageData();PANData = NULL; 

			x=n*width;	
		}
		y=m*height;
		x=0;
	}
	panvar = sqrt(panvar);
	ivar = sqrt(ivar);

	//HSI融合
	int count=1;
	x=0;
	y=0;
	for (m=1;m<=RowBlockNum;m++){
		if (m<RowBlockNum){
			height=BlockHeight;
		}
		else{
			height=MS_Height-(m-1)*BlockHeight; //边界块处理
		}
		for (n=1;n<=ColBlockNum;n++){
			if (n<ColBlockNum){
				width=BlockWidth;
			}
			else{
				width=MS_Width-(n-1)*BlockWidth;//边界块处理
			}
			cout<<"开始处理第 "<<count<<" 块"<<endl;count++;

			if (MSInf->ReadImageToBuff(Input_MS_FileName,x,y,width,height) != 0)//读取MS图像数据
			{
				cerr<<"Read MS Image Data Error."<<endl;
				cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
				exit(1);
			}

			MSData = MSInf->GetImgData();	  //MS图像数据指针
			New_MSData = new float[height*ratio*width*ratio*MS_Bandcount];  //保存MS插值后的影像

			if (NULL == New_MSData)//
			{
				cerr<<"Memory Error.\n";
				cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
				exit(1);
			}
			//插值
			//插值方法：Nearest/Linear/CubicConv
			Interpolation(MSData, height,width ,MS_Bandcount, New_MSData, height*ratio, width*ratio, InterpolationMethod);

			//释放内存
			MSInf->ClearImageData(); MSData = NULL;

			if (PANInf->ReadImageToBuff(Input_PAN_FileName,x*ratio,y*ratio,width*ratio,height*ratio) != 0) //读取PAN图像数据
			{
				cerr<<"Read PAN Image Data Error."<<endl;
				cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
				exit(1);
			}
			PANData = PANInf->GetImgData();    //PAN图像数据指针

			int ROWS = MS_Bandcount;
			int COLS = height*ratio*width*ratio;


			//对MS做HSI变换
			RGB2HSI(New_MSData, height*ratio, width*ratio, MS_Bandcount);

			//对PAN影像灰度拉伸

			for (i = 0; i < COLS; i++)
				PANData[i] = (PANData[i]-panmean)/(panvar);

			for (i = 0; i < COLS; i++)
				PANData[i] = PANData[i]*(ivar)+imean;

			//PAN影像与MS影像融合：用灰度拉伸后的图像替换I分量生成新HSI
			for (i = 0; i < COLS; i++)
				DATA2D(New_MSData, 2, i, COLS) = PANData[i] ; //不需要把PAN图像转换到[0,1]

			//进行PAN影像与MS影像融合
			//  nbscales -- the total number of scales for subband decomposition
			int nbscales=3;
			//  nbangles_coarse -- the number of angles in the 2nd coarest scale
			int nbangles_coarse=4;
			//  ac -- ac==1 use curvelet at the finest level, ac==0, use wavelet at the finest level 
			int ac=1;
			CpxNumMat PanMat(height*ratio,width*ratio);
			for(i=0; i<height*ratio; i++)
				for(j=0; j<width*ratio; j++){
					PanMat(i,j)= cpx(DATA2D(PANData, i, j, width*ratio),0);
					//IMat(i,j) = cpx(DATA2D(I->imgdata, i, j, PANData->width),0);
				}
				//释放内存

			PANInf->ClearImageData();PANData=NULL;

			vector< vector<CpxNumMat> > CPanMat;
			fdct_wrapping(height*ratio,width*ratio, nbscales, nbangles_coarse, ac, PanMat, CPanMat);
			//释放内存
			//PanMat.~NumMat();
			delete[] PanMat._data;PanMat._data=NULL;//clear(PanMat);

			CpxNumMat IMat(height*ratio,width*ratio);
			for(i=0; i<height*ratio; i++)
				for(j=0; j<width*ratio; j++){
					//PanMat(i,j)= cpx(DATA2D(PANData->imgdata, i, j, PANData->width),0);
					IMat(i,j) = cpx(New_MSData[2*COLS+i*width*ratio+j],0);
				}

			vector< vector<CpxNumMat> > CIMat;
			fdct_wrapping(height*ratio,width*ratio, nbscales, nbangles_coarse, ac, IMat, CIMat);

			//释放内存
			//IMat.~NumMat();
			delete[] IMat._data;IMat._data=NULL;//clear(IMat);

			for (i=0;i<CPanMat[0][0].m();i++)
				for (j=0;j<CPanMat[0][0].n();j++){
					CPanMat[0][0](i,j)=0.5*CPanMat[0][0](i,j)+0.5*CIMat[0][0](i,j);
				}
			for (i=1;i<CPanMat.size();i++)
				for (j=0;j<CPanMat[i].size();j++)
					for (p=0;p<CPanMat[i][j].m();p++)
						for (q=0;q<CPanMat[i][j].n();q++){
							CPanMat[i][j](p,q)=	abs(CPanMat[i][j](p,q)) > abs(CIMat[i][j](p,q))? CPanMat[i][j](p,q):CIMat[i][j](p,q);
						}
			//释放内存
			CIMat.clear();

			CpxNumMat Fusion(height*ratio,width*ratio); 
			ifdct_wrapping(height*ratio,width*ratio, nbscales, nbangles_coarse, ac, CPanMat, Fusion);

			//释放内存
			CPanMat.clear();
			//用融合后的图像替换I分量生成新HSI
			for(i=0; i<height*ratio; i++)
				for(j=0; j<width*ratio; j++){

					DATA2D(New_MSData, 2, i*width*ratio+j, COLS) = Fusion(i,j).real() > 0 ? (float)Fusion(i,j).real():0;// 
				}
			//释放内存
			delete[] Fusion._data;Fusion._data=NULL;//clear(Fusion); 

			//将MS从HSI变换到RGB空间
			HSI2RGB(New_MSData, height*ratio,width*ratio, MS_Bandcount);

			//GDAL写文件
			MSInf->SetImgData(New_MSData);
			if(MSInf->WriteImageFromBuff(Output_MS_FileName,x*ratio,y*ratio,width*ratio,height*ratio)!=0){
				cerr<<"Write MS Image Data Error."<<endl;
				cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
				exit(1);
			}
			//释放内存
			MSInf->ClearImageData();New_MSData = NULL;

			x=n*width;	
		}
		y=m*height;
		x=0;
	}
	/************************************************************************/
	/*						分块处理结束                                    */
	/************************************************************************/

	//释放内存
	delete MSInf;	MSInf = NULL;
	delete PANInf;	PANInf = NULL;
}

void CurveletFusion::MeanStd_Curvelet_HIS_Fusion(const char* Input_PAN_FileName, const char* Input_MS_FileName, const char* Output_MS_FileName,const char* LogName,int* bandlist,int InterpolationMethod){
	/*
	 *融合方法：Curvelet与HSI
	 *替换规则：I分量替换
	 *Input_PAN_FileName		高分辨率图像路径及名称
	 *Input_MS_FileName			多光谱图像路径及名称（必须为三个波段）
	 *Output_MS_FileName		融合图像路径及名称
	 *LogName                   日志文件存放路径
	 *bandlist					多光谱或高光谱参与融合的波段顺序
	 *InterpolationMethod		插值方法
	 *作者：YS
	 *日期：2016.2.28
	 */

	GDALAllRegister();         //利用GDAL读取图片，先要进行注册  
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");   //设置支持中文路径 

	try{
		int i,j,p,q;
		Log(LogName,"01|01");//写入log日志

		//读取PAN影像信息
		GdalInf* PANInf = new GdalInf();
		if (NULL == PANInf)
		{
			cerr<<"Memory Error.\n";
			cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
			throw 1;
		}

		if (PANInf->ReadImageInf(Input_PAN_FileName) !=0)
		{
			cerr<<"Read PAN Image Inf Error."<<endl;
			cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
			throw 1;
		}

		//读取MS影像信息
		GdalInf* MSInf = new GdalInf();
		if (NULL == MSInf)
		{
			cerr<<"Memory Error.\n";
			cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
			throw 1;
		}
		if (MSInf->ReadImageInf(Input_MS_FileName)!=0)
		{
			cerr<<"Read MS Image Inf Error."<<endl;
			cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
			throw 1;
		}


		int PAN_Width = PANInf->GetWidth(); //PAN宽度（MS影像插值后宽度）
		int PAN_Height= PANInf->GetHeight();//PAN高度（MS影像插值后高度）

		int MS_Width = MSInf->GetWidth();  //MS宽度
		int MS_Height = MSInf->GetHeight();//MS高度
		int MS_Bandcount = MSInf->GetBandCount(); //MS影像波段数

		if (MS_Bandcount < 3)
		{
			cerr<<"HSI Fusion Error."<<endl;
			cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
			throw 1;
		}
		else if(MS_Bandcount > 3)  //如果波段数大于3 取前3个波段
		{
			MS_Bandcount = 3;
			MSInf->SetBandCount(MS_Bandcount);
		}

		MSInf->SetHeight(PAN_Height); //更新MS影像高度
		MSInf->SetWidth(PAN_Width);	  //更新MS影像宽度
		MSInf->SetGeoTransform(PANInf->GetGeoTransform());//更新地理信息

		float* PANData = NULL;    //记录PAN数据
		float* MSData = NULL;    //记录MS原始数据
		float* New_MSData = NULL;  //记录MS插值后数据

		if (MSInf->ReadImageToBuff(Input_MS_FileName,0,0,MS_Width,MS_Height,bandlist) != 0)//读取MS图像数据
		{
			cerr<<"Read MS Image Data Error."<<endl;
			cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
			throw 1;
		}

		Log(LogName,"01|02");//写入log日志

		//MS图像数据指针
		MSData = MSInf->GetImgData();	  
		New_MSData = new float[PAN_Height*PAN_Width*MS_Bandcount];  //保存MS插值后的影像
		if (NULL == New_MSData)
		{
			cerr<<"Memory Error.\n";
			cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
			throw 2;
		}
		//插值
		//插值方法：Nearest/Linear/CubicConv

		Interpolation(MSData, MS_Height,MS_Width ,MS_Bandcount, New_MSData, PAN_Height, PAN_Width, InterpolationMethod);

		//释放内存
		MSInf->ClearImageData(); MSData = NULL;

		//读取PAN图像数据
		if (PANInf->ReadImageToBuff(Input_PAN_FileName,0,0,PAN_Width,PAN_Height) != 0) 
		{

			cerr<<"Read PAN Image Data Error."<<endl;
			cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
			throw 1;
		}

		PANData = PANInf->GetImgData();    //PAN图像数据指针

		Log(LogName,"01|03");//写入log日志

		int ROWS = MS_Bandcount;
		int COLS = PAN_Width*PAN_Height;

		//对MS做HSI变换
		RGB2HSI(New_MSData, PAN_Height, PAN_Width, MS_Bandcount);

		//对PAN影像灰度拉伸

		float mean,s;//均值 标准差

		MeanStd(PANData,PAN_Height, PAN_Width,s,mean);

		for (i = 0; i < COLS; i++)
			PANData[i] = (PANData[i]-mean)/(s);

		MeanStd(New_MSData+2*COLS,PAN_Height, PAN_Width,s,mean); 

		for (i = 0; i < COLS; i++)
			PANData[i] = (PANData[i]*(s)+mean) > 0? (PANData[i]*(s)+mean) : 0;


		//进行PAN影像与MS影像融合

		//  nbscales -- the total number of scales for subband decomposition
		int nbscales=3;
		//  nbangles_coarse -- the number of angles in the 2nd coarest scale
		int nbangles_coarse=4;
		//  ac -- ac==1 use curvelet at the finest level, ac==0, use wavelet at the finest level 
		int ac=1;
		CpxNumMat PanMat(PAN_Height, PAN_Width);
		for(i=0; i<PAN_Height; i++)
			for(j=0; j<PAN_Width; j++){
				PanMat(i,j)= cpx(DATA2D(PANData, i, j, PAN_Width),0);
				//IMat(i,j) = cpx(DATA2D(I->imgdata, i, j, PANData->width),0);
			}
		//释放内存

		PANInf->ClearImageData();PANData=NULL;

		vector< vector<CpxNumMat> > CPanMat;
		fdct_wrapping(PAN_Height, PAN_Width, nbscales, nbangles_coarse, ac, PanMat, CPanMat);
		//释放内存
		//PanMat.~NumMat();
		delete[] PanMat._data;PanMat._data=NULL;//clear(PanMat);

		CpxNumMat IMat(PAN_Height, PAN_Width);
		for(i=0; i<PAN_Height; i++)
			for(j=0; j<PAN_Width; j++){
				//PanMat(i,j)= cpx(DATA2D(PANData->imgdata, i, j, PANData->width),0);
				IMat(i,j) = cpx(New_MSData[2*COLS+i*PAN_Width+j],0);
			}

		vector< vector<CpxNumMat> > CIMat;
		fdct_wrapping(PAN_Height, PAN_Width, nbscales, nbangles_coarse, ac, IMat, CIMat);

		//释放内存
		//IMat.~NumMat();
		delete[] IMat._data;IMat._data=NULL;//clear(IMat);

		for (i=0;i<CPanMat[0][0].m();i++)
			for (j=0;j<CPanMat[0][0].n();j++){
				CPanMat[0][0](i,j)=0.5*CPanMat[0][0](i,j)+0.5*CIMat[0][0](i,j);
			}
		for (i=1;i<CPanMat.size();i++)
			for (j=0;j<CPanMat[i].size();j++)
				for (p=0;p<CPanMat[i][j].m();p++)
					for (q=0;q<CPanMat[i][j].n();q++){
						CPanMat[i][j](p,q)=	abs(CPanMat[i][j](p,q)) > abs(CIMat[i][j](p,q))? CPanMat[i][j](p,q):CIMat[i][j](p,q);
					}
		//释放内存
		CIMat.clear();

		CpxNumMat Fusion(PAN_Height, PAN_Width); 
		ifdct_wrapping(PAN_Height, PAN_Width, nbscales, nbangles_coarse, ac, CPanMat, Fusion);

		//释放内存
		CPanMat.clear();
		//用融合后的图像替换I分量生成新HCS
		for(i=0; i<PAN_Height; i++)
			for(j=0; j<PAN_Width; j++){

				DATA2D(New_MSData, 2, i*PAN_Width+j, COLS) = Fusion(i,j).real() > 0 ? (float)Fusion(i,j).real():0;// 
			}
		//释放内存
		delete[] Fusion._data;Fusion._data=NULL;//clear(Fusion);

		//将MS从HSI变换到RGB空间
		HSI2RGB(New_MSData, PAN_Height, PAN_Width, MS_Bandcount);

		//GDAL写文件
		Log(LogName,"01|04");//写入log日志

		if (MSInf->WriteImageInf(Output_MS_FileName)!=0) //创建文件，写入头文件信息
		{
			cerr<<"Write MS Image Inf Error."<<endl;
			cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
			throw 4;
		}

		MSInf->SetImgData(New_MSData);

		if(MSInf->WriteImageFromBuff(Output_MS_FileName,0,0,PAN_Width,PAN_Height)!=0){
			cerr<<"Read PAN Image Data Error."<<endl;
			cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
			throw 4;
		}
		//释放内存
		MSInf->ClearImageData();
		PANData = NULL;    
		MSData = NULL;  
		New_MSData = NULL;

		//释放内存
		delete MSInf;	MSInf = NULL;
		delete PANInf;	PANInf = NULL;

		Log(LogName,"01|05");//写入log日志
	}
	catch(int e){
		if (e == 1)
		{
			Log(LogName,"02|01");//写入log日志
		}
		else if (e == 2)
		{
			Log(LogName,"02|02");//写入log日志
		}
		else if (e == 3)
		{
			Log(LogName,"02|03");//写入log日志
		}
		else if (e == 4)
		{
			Log(LogName,"02|04");//写入log日志
		}
		else{
			//Log(LogName,"02|03");//写入log日志
		}
		return;
	}
	catch(char *str){
		//linux下 fftw malloc.c exit(EXIT_FAILURE);改为throw "fftw error";
		//windows下 会出错
		Log(LogName,"02|03");//写入log日志
		return;
	}
}

void CurveletFusion::MeanStd_Curvelet_HCS_Fusion(const char* Input_PAN_FileName, const char* Input_MS_FileName, const char* Output_MS_FileName,const char* LogName,int* bandlist,int InterpolationMethod){
	/*
	 *融合方法：Curvelet与HCS
	 *替换规则：I分量替换
	 *Input_PAN_FileName		高分辨率图像路径及名称
	 *Input_MS_FileName			多光谱图像路径及名称（必须为三个波段）
	 *Output_MS_FileName		融合图像路径及名称
	 *LogName                   日志文件存放路径
	 *bandlist					多光谱或高光谱参与融合的波段顺序
	 *InterpolationMethod		插值方法
	 *作者：YS
	 *日期：2016.3.7
	 */

	GDALAllRegister();         //利用GDAL读取图片，先要进行注册  
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");   //设置支持中文路径 

	try{
		int i,j,p,q;
		Log(LogName,"01|01");//写入log日志

		//读取PAN影像信息
		GdalInf* PANInf = new GdalInf();
		if (NULL == PANInf)
		{
			cerr<<"Memory Error.\n";
			cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
			throw 1;
		}

		if (PANInf->ReadImageInf(Input_PAN_FileName) !=0)
		{
			cerr<<"Read PAN Image Inf Error."<<endl;
			cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
			throw 1;
		}

		//读取MS影像信息
		GdalInf* MSInf = new GdalInf();
		if (NULL == MSInf)
		{
			cerr<<"Memory Error.\n";
			cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
			throw 1;
		}
		if (MSInf->ReadImageInf(Input_MS_FileName)!=0)
		{
			cerr<<"Read MS Image Inf Error."<<endl;
			cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
			throw 1;
		}


		int PAN_Width = PANInf->GetWidth(); //PAN宽度（MS影像插值后宽度）
		int PAN_Height= PANInf->GetHeight();//PAN高度（MS影像插值后高度）

		int MS_Width = MSInf->GetWidth();  //MS宽度
		int MS_Height = MSInf->GetHeight();//MS高度
		int MS_Bandcount = MSInf->GetBandCount(); //MS影像波段数

		if (MS_Bandcount < 3)
		{
			cerr<<"Curvelet_HCS Fusion Error."<<endl;
			cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
			throw 1;
		}
		else if(MS_Bandcount > 3)  //如果波段数大于3 取前3个波段
		{
			MS_Bandcount = 3;
			MSInf->SetBandCount(MS_Bandcount);
		}

		MSInf->SetHeight(PAN_Height); //更新MS影像高度
		MSInf->SetWidth(PAN_Width);	  //更新MS影像宽度
		MSInf->SetGeoTransform(PANInf->GetGeoTransform());//更新地理信息

		float* PANData = NULL;    //记录PAN数据
		float* MSData = NULL;    //记录MS原始数据
		float* New_MSData = NULL;  //记录MS插值后数据

		if (MSInf->ReadImageToBuff(Input_MS_FileName,0,0,MS_Width,MS_Height,bandlist) != 0)//读取MS图像数据
		{
			cerr<<"Read MS Image Data Error."<<endl;
			cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
			throw 1;
		}

		Log(LogName,"01|02");//写入log日志

		//MS图像数据指针
		MSData = MSInf->GetImgData();	  
		New_MSData = new float[PAN_Height*PAN_Width*MS_Bandcount];  //保存MS插值后的影像
		if (NULL == New_MSData)
		{
			cerr<<"Memory Error.\n";
			cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
			throw 2;
		}
		//插值
		//插值方法：Nearest/Linear/CubicConv

		Interpolation(MSData, MS_Height,MS_Width ,MS_Bandcount, New_MSData, PAN_Height, PAN_Width, InterpolationMethod);

		//释放内存
		MSInf->ClearImageData(); MSData = NULL;

		//读取PAN图像数据
		if (PANInf->ReadImageToBuff(Input_PAN_FileName,0,0,PAN_Width,PAN_Height) != 0) 
		{

			cerr<<"Read PAN Image Data Error."<<endl;
			cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
			throw 1;
		}

		PANData = PANInf->GetImgData();    //PAN图像数据指针

		Log(LogName,"01|03");//写入log日志

		int ROWS = MS_Bandcount;
		int COLS = PAN_Width*PAN_Height;

		//对MS做HSI变换
		HCS_Trans(New_MSData, PAN_Height, PAN_Width, MS_Bandcount);

		//对PAN影像灰度拉伸

		float mean,s;//均值 标准差

		for (i = 0; i < COLS; i++){
			PANData[i] = (PANData[i]*PANData[i]);
			DATA2D(New_MSData, 0, i, COLS) = DATA2D(New_MSData, 0, i, COLS)*DATA2D(New_MSData, 0, i, COLS);
		}

		MeanStd(PANData,PAN_Height, PAN_Width,s,mean);

		for (i = 0; i < COLS; i++)
			PANData[i] = (PANData[i]-mean+s)/(s);

		MeanStd(New_MSData,PAN_Height, PAN_Width,s,mean); 

		for (i = 0; i < COLS; i++){
			PANData[i] = PANData[i]*(s)+mean-s > 0 ? sqrt(PANData[i]*(s)+mean-s) : 0; //截断
			DATA2D(New_MSData, 0, i, COLS) = sqrt(DATA2D(New_MSData, 0, i, COLS));    //开平方 恢复原来的图像

		}


		//进行PAN影像与MS影像融合

		//  nbscales -- the total number of scales for subband decomposition
		int nbscales=3;
		//  nbangles_coarse -- the number of angles in the 2nd coarest scale
		int nbangles_coarse=4;
		//  ac -- ac==1 use curvelet at the finest level, ac==0, use wavelet at the finest level 
		int ac=1;
		CpxNumMat PanMat(PAN_Height, PAN_Width);
		for(i=0; i<PAN_Height; i++)
			for(j=0; j<PAN_Width; j++){
				PanMat(i,j)= cpx(DATA2D(PANData, i, j, PAN_Width),0);
				//IMat(i,j) = cpx(DATA2D(I->imgdata, i, j, PANData->width),0);
			}
		//释放内存

		PANInf->ClearImageData();PANData=NULL;

		vector< vector<CpxNumMat> > CPanMat;
		fdct_wrapping(PAN_Height, PAN_Width, nbscales, nbangles_coarse, ac, PanMat, CPanMat);
		//释放内存
		//PanMat.~NumMat();
		delete[] PanMat._data;PanMat._data=NULL;//clear(PanMat);

		CpxNumMat IMat(PAN_Height, PAN_Width);
		for(i=0; i<PAN_Height; i++)
			for(j=0; j<PAN_Width; j++){
				//PanMat(i,j)= cpx(DATA2D(PANData->imgdata, i, j, PANData->width),0);
				IMat(i,j) = cpx(New_MSData[i*PAN_Width+j],0);  //第一行为HCS变换后的I分量
			}

		vector< vector<CpxNumMat> > CIMat;
		fdct_wrapping(PAN_Height, PAN_Width, nbscales, nbangles_coarse, ac, IMat, CIMat);

		//释放内存
		//IMat.~NumMat();
		delete[] IMat._data;IMat._data=NULL;//clear(IMat);

		for (i=0;i<CPanMat[0][0].m();i++)
			for (j=0;j<CPanMat[0][0].n();j++){
				CPanMat[0][0](i,j)=0.5*CPanMat[0][0](i,j)+0.5*CIMat[0][0](i,j);
			}
		for (i=1;i<CPanMat.size();i++)
			for (j=0;j<CPanMat[i].size();j++)
				for (p=0;p<CPanMat[i][j].m();p++)
					for (q=0;q<CPanMat[i][j].n();q++){
						CPanMat[i][j](p,q)=	abs(CPanMat[i][j](p,q)) > abs(CIMat[i][j](p,q))? CPanMat[i][j](p,q):CIMat[i][j](p,q);
					}
		//释放内存
		CIMat.clear();

		CpxNumMat Fusion(PAN_Height, PAN_Width); 
		ifdct_wrapping(PAN_Height, PAN_Width, nbscales, nbangles_coarse, ac, CPanMat, Fusion);

		//释放内存
		CPanMat.clear();
		//用融合后的图像替换I分量生成新HCS
		for(i=0; i<PAN_Height; i++)
			for(j=0; j<PAN_Width; j++){

				DATA2D(New_MSData, 0, i*PAN_Width+j, COLS) = Fusion(i,j).real() > 0 ? (float)Fusion(i,j).real():0;// 
			}
		//释放内存
		delete[] Fusion._data;Fusion._data=NULL;//clear(Fusion);

		//将MS从HSI变换到RGB空间
		HCS_Itrans(New_MSData, PAN_Height, PAN_Width, MS_Bandcount);

		//GDAL写文件
		Log(LogName,"01|04");//写入log日志

		if (MSInf->WriteImageInf(Output_MS_FileName)!=0) //创建文件，写入头文件信息
		{
			cerr<<"Write MS Image Inf Error."<<endl;
			cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
			throw 4;
		}

		MSInf->SetImgData(New_MSData);

		if(MSInf->WriteImageFromBuff(Output_MS_FileName,0,0,PAN_Width,PAN_Height)!=0){
			cerr<<"Read PAN Image Data Error."<<endl;
			cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
			throw 4;
		}
		//释放内存
		MSInf->ClearImageData();
		PANData = NULL;    
		MSData = NULL;  
		New_MSData = NULL;

		//释放内存
		delete MSInf;	MSInf = NULL;
		delete PANInf;	PANInf = NULL;

		Log(LogName,"01|05");//写入log日志
	}
	catch(int e){
		if (e == 1)
		{
			Log(LogName,"02|01");//写入log日志
		}
		else if (e == 2)
		{
			Log(LogName,"02|02");//写入log日志
		}
		else if (e == 3)
		{
			Log(LogName,"02|03");//写入log日志
		}
		else if (e == 4)
		{
			Log(LogName,"02|04");//写入log日志
		}
		else{
			//Log(LogName,"02|03");//写入log日志
		}
		return;
	}
	catch(char *str){
		//linux下 fftw malloc.c exit(EXIT_FAILURE);改为throw "fftw error";
		//windows下 会出错
		Log(LogName,"02|03");//写入log日志
		return;
	}

}

void CurveletFusion::MeanStd_Curvelet_GramSchmidt_Fusion(const char* Input_PAN_FileName, const char* Input_MS_FileName, const char* Output_MS_FileName,const char* LogName,int* bandlist,int InterpolationMethod){
	/*
	 *融合方法：MeanStd_Curvelet_GramSchmidt_Fusion（用多光谱波段影像模拟全色图像，取均值）
	 *Input_PAN_FileName		高分辨率图像路径及名称
	 *Input_MS_FileName			多光谱图像路径及名称（必须为三个波段）
	 *Output_MS_FileName		融合图像路径及名称
	 *LogName                   日志文件存放路径
	 *bandlist					多光谱或高光谱参与融合的波段顺序
	 *InterpolationMethod		插值方法
	 *作者：YS
	 *日期：2016.3.1
	 */
	

	GDALAllRegister();         //利用GDAL读取图片，先要进行注册  
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");   //设置支持中文路径 
	try{
		int i,j,k,p,q;
		Log(LogName,"01|01");//写入log日志

		//读取PAN影像信息
		GdalInf* PANInf = new GdalInf();
		if (NULL == PANInf)
		{
			cerr<<"Memory Error.\n";
			cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
			throw 1;
		}

		if (PANInf->ReadImageInf(Input_PAN_FileName) !=0)
		{
			cerr<<"Read PAN Image Inf Error."<<endl;
			cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
			throw 1;
		}
	
		//读取MS影像信息
		GdalInf* MSInf = new GdalInf();
		if (NULL == MSInf)
		{
			cerr<<"Memory Error.\n";
			cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
			throw 1;
		}
		if (MSInf->ReadImageInf(Input_MS_FileName)!=0)
		{
			cerr<<"Read MS Image Inf Error."<<endl;
			cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
			throw 1;
		}
	
	
		int PAN_Width = PANInf->GetWidth(); //PAN宽度（MS影像插值后宽度）
		int PAN_Height= PANInf->GetHeight();//PAN高度（MS影像插值后高度）
	
		int MS_Width = MSInf->GetWidth();  //MS宽度
		int MS_Height = MSInf->GetHeight();//MS高度
		int MS_Bandcount = MSInf->GetBandCount(); //MS影像波段数
	
		if (MS_Bandcount < 3)
		{
			cerr<<"Curvelet_GramSchmidt Fusion Error."<<endl;
			cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
			throw 1;
		}
		else if(MS_Bandcount > 3) //如果波段数大于3 取前3个波段
		{
			MS_Bandcount = 3;
			MSInf->SetBandCount(MS_Bandcount);
		}

		MSInf->SetHeight(PAN_Height); //更新MS影像高度
		MSInf->SetWidth(PAN_Width);	  //更新MS影像宽度
		MSInf->SetGeoTransform(PANInf->GetGeoTransform());//更新地理信息

		float* PANData =NULL;    //记录PAN数据
		float* MSData = NULL;    //记录MS原始数据
		float* New_MSData=NULL;  //记录MS插值后数据
	
		if (MSInf->ReadImageToBuff(Input_MS_FileName,0,0,MS_Width,MS_Height,bandlist) != 0)//读取MS图像数据
		{
			cerr<<"Read MS Image Data Error."<<endl;
			cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
			throw 1;
		}

		//MS图像数据指针
	
		Log(LogName,"01|02");//写入log日志

		MSData = MSInf->GetImgData();	  
		New_MSData = new float[PAN_Height*PAN_Width*(MS_Bandcount+1)];  //保存MS插值后的影像
		if (NULL == New_MSData)
		{
			cerr<<"Memory Error.\n";
			cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
			throw 2;
		}
		//插值
		//插值方法：Nearest/Linear/CubicConv
	
		Interpolation(MSData, MS_Height,MS_Width ,MS_Bandcount, New_MSData+PAN_Height*PAN_Width, PAN_Height, PAN_Width, InterpolationMethod);

		//释放内存
		MSInf->ClearImageData(); MSData = NULL;

		//读取PAN图像数据
		if (PANInf->ReadImageToBuff(Input_PAN_FileName,0,0,PAN_Width,PAN_Height) != 0) 
		{
			cerr<<"Read PAN Image Data Error."<<endl;
			cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
			throw 1;
		}

		PANData = PANInf->GetImgData();    //PAN图像数据指针


		int ROWS = MS_Bandcount+1;
		int COLS = PAN_Width*PAN_Height;


		//对MS做GramSchmidt融合 
		Log(LogName,"01|03");//写入log日志
		//通过取MS的平均值模拟全色
		
		double sum;
		for(i=0;i<COLS;i++){
			sum = 0.0;
			for(j=1;j<ROWS;j++)
				//sum += DATA2D(New_MSData,j,i,COLS);  //1)
				sum += pow(DATA2D(New_MSData,j,i,COLS),2); //2)
				
			//DATA2D(New_MSData,0,i,COLS) = sum/MS_Bandcount;//1)
			DATA2D(New_MSData,0,i,COLS) = sqrt(sum/MS_Bandcount);//2) 比1）降低了偏差
			//DATA2D(New_MSData,0,i,COLS) = sqrt(sum)/MS_Bandcount;//3)效果不好
		}	
		
		float *R = new float[ROWS*ROWS];
		float *m = new float[ROWS];
		if (NULL == R ||NULL == m)
		{
			cerr<<"RGramSchmidt Error."<<endl;
			cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
			throw 3;
		}
		for (i=0;i<ROWS;i++)
		{
			R[i*MS_Bandcount+i] = Cov(New_MSData+(i)*COLS,New_MSData+(i)*COLS,COLS);

			R[i*MS_Bandcount+i] = R[i*MS_Bandcount+i]*R[i*MS_Bandcount+i];

			for (j=i+1;j<ROWS;j++)
			{
				R[i*MS_Bandcount+j] = Cov(New_MSData+(i)*COLS,New_MSData+(j)*COLS,COLS);
			}
			m[i] = Mean(New_MSData+(i)*COLS,COLS);
			//cout<<m[i]<<endl;
			
			for (j=0;j<COLS;j++)
			{			

				DATA2D(New_MSData,i,j,COLS) -= m[i];

			}
			for (j=0;j<COLS;j++)
			{			
				for (k=0;k<i;k++)
				{
					DATA2D(New_MSData,i,j,COLS) -= (DATA2D(New_MSData,k,j,COLS)*R[k*MS_Bandcount+i]/R[k*MS_Bandcount+k]);
				}
			}
		}
		//拉伸
		
		float pan_mean,pan_std;
		float ms_mean,ms_std;

		MeanStd(PANData,PAN_Height, PAN_Width,pan_std,pan_mean);

		MeanStd(New_MSData,PAN_Height, PAN_Width,ms_std,ms_mean); 

		for (i = 0; i < COLS; i++)
			PANData[i] = ((PANData[i]-pan_mean)/pan_std) * ms_std + ms_mean;

		//进行PAN影像与MS影像融合

		//  nbscales -- the total number of scales for subband decomposition
		int nbscales=3;
		//  nbangles_coarse -- the number of angles in the 2nd coarest scale
		int nbangles_coarse=4;
		//  ac -- ac==1 use curvelet at the finest level, ac==0, use wavelet at the finest level 
		int ac=1;
		CpxNumMat PanMat(PAN_Height, PAN_Width);
		for(i=0; i<PAN_Height; i++)
			for(j=0; j<PAN_Width; j++){
				PanMat(i,j)= cpx(DATA2D(PANData, i, j, PAN_Width),0);
				//IMat(i,j) = cpx(DATA2D(I->imgdata, i, j, PANData->width),0);
			}
		//释放内存

		PANInf->ClearImageData();PANData=NULL;

		vector< vector<CpxNumMat> > CPanMat;
		fdct_wrapping(PAN_Height, PAN_Width, nbscales, nbangles_coarse, ac, PanMat, CPanMat);
		//释放内存
		//PanMat.~NumMat();
		delete[] PanMat._data;PanMat._data=NULL;//clear(PanMat);

		CpxNumMat IMat(PAN_Height, PAN_Width);
		for(i=0; i<PAN_Height; i++)
			for(j=0; j<PAN_Width; j++){
				//PanMat(i,j)= cpx(DATA2D(PANData->imgdata, i, j, PANData->width),0);
				IMat(i,j) = cpx(New_MSData[0*COLS+i*PAN_Width+j],0);
			}

		vector< vector<CpxNumMat> > CIMat;
		fdct_wrapping(PAN_Height, PAN_Width, nbscales, nbangles_coarse, ac, IMat, CIMat);

		//释放内存
		//IMat.~NumMat();
		delete[] IMat._data;IMat._data=NULL;//clear(IMat);

		for (i=0;i<CPanMat[0][0].m();i++)
			for (j=0;j<CPanMat[0][0].n();j++){
				CPanMat[0][0](i,j)=0.5*CPanMat[0][0](i,j)+0.5*CIMat[0][0](i,j);
			}
		for (i=1;i<CPanMat.size();i++)
			for (j=0;j<CPanMat[i].size();j++)
				for (p=0;p<CPanMat[i][j].m();p++)
					for (q=0;q<CPanMat[i][j].n();q++){
						CPanMat[i][j](p,q)=	abs(CPanMat[i][j](p,q)) > abs(CIMat[i][j](p,q))? CPanMat[i][j](p,q):CIMat[i][j](p,q);
					}
		//释放内存
		CIMat.clear();

		CpxNumMat Fusion(PAN_Height, PAN_Width); 
		ifdct_wrapping(PAN_Height, PAN_Width, nbscales, nbangles_coarse, ac, CPanMat, Fusion);

		//释放内存
		CPanMat.clear();
		//用融合后的图像替换
		for(i=0; i<PAN_Height; i++)
			for(j=0; j<PAN_Width; j++){

				DATA2D(New_MSData, 0, i*PAN_Width+j, COLS) = Fusion(i,j).real() > 0 ? (float)Fusion(i,j).real():0;// 
			}
		//释放内存
		delete[] Fusion._data;Fusion._data = NULL;//clear(Fusion);

		////替换
		//for(i=0;i<COLS;i++){
		//	DATA2D(New_MSData,0,i,COLS) = (PANData[i]);
		//	//DATA2D(New_MSData,0,i,COLS) = (PANData[i]+DATA2D(New_MSData,0,i,COLS))/2;
		//}

		//反变换
		for (i=ROWS-1;i>=0;i--)
		{
			for (j=0;j<COLS;j++)
			{			

				DATA2D(New_MSData,i,j,COLS) += m[i];

			}

			for (j=0;j<COLS;j++)
			{			
				for (k=0;k<i;k++)
				{
					DATA2D(New_MSData,i,j,COLS) += (DATA2D(New_MSData,k,j,COLS)*R[k*MS_Bandcount+i]/R[k*MS_Bandcount+k]);
				}
				DATA2D(New_MSData,i,j,COLS) = DATA2D(New_MSData,i,j,COLS)>0 ? DATA2D(New_MSData,i,j,COLS):0; //截断
			}
		}

		//释放内存
		PANInf->ClearImageData();PANData = NULL;

		//GDAL写文件
		Log(LogName,"01|04");//写入log日志

		if (MSInf->WriteImageInf(Output_MS_FileName)!=0) //创建文件，写入头文件信息
		{
			cerr<<"Write MS Image Inf Error."<<endl;
			cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
			throw 4;
		}

		MSInf->SetImgData((New_MSData+PAN_Height*PAN_Width));
		if(MSInf->WriteImageFromBuff(Output_MS_FileName,0,0,PAN_Width,PAN_Height)!=0){
			cerr<<"Write MS Image Data Error."<<endl;
			cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
			throw 4;
		}
		//释放内存
		delete[] New_MSData ;New_MSData = NULL;
		//MSInf->ClearImageData();

	
		//释放内存
		//delete MSInf;	MSInf = NULL;
		delete PANInf;	PANInf = NULL;

		Log(LogName,"01|05");//写入log日志
	}
	catch(int e){
		if (e == 1)
		{
			Log(LogName,"02|01");//写入log日志
		}
		else if (e == 2)
		{
			Log(LogName,"02|02");//写入log日志
		}
		else if (e == 3)
		{
			Log(LogName,"02|03");//写入log日志
		}
		else if (e == 4)
		{
			Log(LogName,"02|04");//写入log日志
		}
		else{
			//Log(LogName,"02|03");//写入log日志
		}
		return;
	}
	catch(char *str){
		Log(LogName,"02|03");//写入log日志
		return;
	}
	

}

//改进 2016.3.17
void CurveletFusion::MeanStd_Curvelet_HCS_Fusion_New(const char* Input_PAN_FileName,
                                                     const char* Input_MS_FileName,
                                                     const char* Output_MS_FileName,
                                                     const char* LogName,
                                                     int* bandlist,int InterpolationMethod) {
	/*
	 *融合方法：Curvelet与HCS
	 *替换规则：I分量替换
	 *Input_PAN_FileName		高分辨率图像路径及名称
	 *Input_MS_FileName			多光谱图像路径及名称（必须为三个波段）
	 *Output_MS_FileName		融合图像路径及名称
	 *LogName                   日志文件存放路径
	 *bandlist					多光谱或高光谱参与融合的波段顺序
	 *InterpolationMethod		插值方法
	 *作者：YS
	 *日期：2016.3.7
	 */

	GDALAllRegister();         //利用GDAL读取图片，先要进行注册  
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");   //设置支持中文路径 

	try{
		int i,j,p,q;
		Log(LogName,"01|01");//写入log日志

		//读取PAN影像信息
		GdalInf* PANInf = new GdalInf();
		if (NULL == PANInf)
		{
			cerr<<"Memory Error.\n";
			cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
			throw 1;
		}

		if (PANInf->ReadImageInf(Input_PAN_FileName) !=0)
		{
			cerr<<"Read PAN Image Inf Error."<<endl;
			cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
			throw 1;
		}

		//读取MS影像信息
		GdalInf* MSInf = new GdalInf();
		if (NULL == MSInf)
		{
			cerr<<"Memory Error.\n";
			cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
			throw 1;
		}
		if (MSInf->ReadImageInf(Input_MS_FileName)!=0)
		{
			cerr<<"Read MS Image Inf Error."<<endl;
			cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
			throw 1;
		}


		int PAN_Width = PANInf->GetWidth(); //PAN宽度（MS影像插值后宽度）
		int PAN_Height= PANInf->GetHeight();//PAN高度（MS影像插值后高度）

		int MS_Width = MSInf->GetWidth();  //MS宽度
		int MS_Height = MSInf->GetHeight();//MS高度
		int MS_Bandcount = MSInf->GetBandCount(); //MS影像波段数

		if (MS_Bandcount < 3)
		{
			cerr<<"Curvelet_HCS Fusion Error."<<endl;
			cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
			throw 1;
		}
		else if(MS_Bandcount > 3)  //如果波段数大于3 取前3个波段
		{
			MS_Bandcount = 3;
			MSInf->SetBandCount(MS_Bandcount);
		}

		MSInf->SetHeight(PAN_Height); //更新MS影像高度
		MSInf->SetWidth(PAN_Width);	  //更新MS影像宽度
		MSInf->SetGeoTransform(PANInf->GetGeoTransform());//更新地理信息

		float* PANData = NULL;    //记录PAN数据
		float* MSData = NULL;    //记录MS原始数据
		float* New_MSData = NULL;  //记录MS插值后数据

		if (MSInf->ReadImageToBuff(Input_MS_FileName,0,0,MS_Width,MS_Height,bandlist) != 0)//读取MS图像数据
		{
			cerr<<"Read MS Image Data Error."<<endl;
			cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
			throw 1;
		}

		Log(LogName,"01|02");//写入log日志

		//MS图像数据指针
		MSData = MSInf->GetImgData();	  
		New_MSData = new float[PAN_Height*PAN_Width*MS_Bandcount];  //保存MS插值后的影像
		if (NULL == New_MSData)
		{
			cerr<<"Memory Error.\n";
			cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
			throw 2;
		}
		//插值
		//插值方法：Nearest/Linear/CubicConv

		Interpolation(MSData, MS_Height,MS_Width ,MS_Bandcount, New_MSData, PAN_Height, PAN_Width, InterpolationMethod);

		//释放内存
		MSInf->ClearImageData(); MSData = NULL;

		//读取PAN图像数据
		if (PANInf->ReadImageToBuff(Input_PAN_FileName,0,0,PAN_Width,PAN_Height) != 0) 
		{

			cerr<<"Read PAN Image Data Error."<<endl;
			cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
			throw 1;
		}

		PANData = PANInf->GetImgData();    //PAN图像数据指针

		Log(LogName,"01|03");//写入log日志

		int ROWS = MS_Bandcount;
		int COLS = PAN_Width*PAN_Height;

		//对MS做HSI变换
		HCS_Trans(New_MSData, PAN_Height, PAN_Width, MS_Bandcount);

		//对PAN影像灰度拉伸

		float mean,s;//均值 标准差

		for (i = 0; i < COLS; i++){
			PANData[i] = (PANData[i]*PANData[i]);
			DATA2D(New_MSData, 0, i, COLS) = DATA2D(New_MSData, 0, i, COLS)*DATA2D(New_MSData, 0, i, COLS);
		}

		MeanStd(PANData,PAN_Height, PAN_Width,s,mean);

		for (i = 0; i < COLS; i++)
			PANData[i] = (PANData[i]-mean+s)/(s);

		MeanStd(New_MSData,PAN_Height, PAN_Width,s,mean); 

		for (i = 0; i < COLS; i++){
			PANData[i] = PANData[i]*(s)+mean-s > 0 ? sqrt(PANData[i]*(s)+mean-s) : 0; //截断
			DATA2D(New_MSData, 0, i, COLS) = sqrt(DATA2D(New_MSData, 0, i, COLS));    //开平方 恢复原来的图像

		}


		//进行PAN影像与MS影像融合

		//  nbscales -- the total number of scales for subband decomposition
		int nbscales=4;
		//  nbangles_coarse -- the number of angles in the 2nd coarest scale
		int nbangles_coarse=4;
		//  ac -- ac==1 use curvelet at the finest level, ac==0, use wavelet at the finest level 
		int ac=1;
		CpxNumMat PanMat(PAN_Height, PAN_Width);
		for(i=0; i<PAN_Height; i++)
			for(j=0; j<PAN_Width; j++){
				PanMat(i,j)= cpx(DATA2D(PANData, i, j, PAN_Width),0);
				//IMat(i,j) = cpx(DATA2D(I->imgdata, i, j, PANData->width),0);
			}
		//释放内存

		PANInf->ClearImageData();PANData=NULL;

		vector< vector<CpxNumMat> > CPanMat;
		fdct_wrapping(PAN_Height, PAN_Width, nbscales, nbangles_coarse, ac, PanMat, CPanMat);
		//释放内存
		//PanMat.~NumMat();
		delete[] PanMat._data;PanMat._data=NULL;//clear(PanMat);

		CpxNumMat IMat(PAN_Height, PAN_Width);
		for(i=0; i<PAN_Height; i++)
			for(j=0; j<PAN_Width; j++){
				//PanMat(i,j)= cpx(DATA2D(PANData->imgdata, i, j, PANData->width),0);
				IMat(i,j) = cpx(New_MSData[i*PAN_Width+j],0);  //第一行为HCS变换后的I分量
			}

		vector< vector<CpxNumMat> > CIMat;
		fdct_wrapping(PAN_Height, PAN_Width, nbscales, nbangles_coarse, ac, IMat, CIMat);

		//释放内存
		//IMat.~NumMat();
		delete[] IMat._data;IMat._data=NULL;//clear(IMat);

		float pan_mean,pan_std,pan_entropy;
		float ms_mean,ms_std,ms_entropy;
		int localSize = 5;
		float x1,x2;
		for (i=0;i<CPanMat[0][0].m();i++)
			for (j=0;j<CPanMat[0][0].n();j++){

				LocalMeanStdEntropy(CPanMat[0][0],i,j,localSize,pan_mean,pan_std,pan_entropy);
				LocalMeanStdEntropy(CIMat[0][0],i,j,localSize,ms_mean,ms_std,ms_entropy);
				x1 = ((pan_std+ms_std)+abs(pan_std-ms_std))/(2*sqrt(pan_std*pan_std+ms_std*ms_std));
				x2 = ((pan_std+ms_std)-abs(pan_std-ms_std))/(2*sqrt(pan_std*pan_std+ms_std*ms_std));
				
				if(pan_entropy > ms_entropy){
					CPanMat[0][0](i,j)=cpx(x1*CPanMat[0][0](i,j).real()+x2*CIMat[0][0](i,j).real(),x1*CPanMat[0][0](i,j).imag()+x2*CIMat[0][0](i,j).imag());
				}
				else{
					CPanMat[0][0](i,j)=cpx(x2*CPanMat[0][0](i,j).real()+x1*CIMat[0][0](i,j).real(),x2*CPanMat[0][0](i,j).imag()+x1*CIMat[0][0](i,j).imag());

				}

			}
		for (i=1;i<CPanMat.size();i++)
			for (j=0;j<CPanMat[i].size();j++)
				for (p=0;p<CPanMat[i][j].m();p++)
					for (q=0;q<CPanMat[i][j].n();q++){
						//CPanMat[i][j](p,q) = cpx(0.5*CPanMat[i][j](p,q).real()+0.5*CIMat[i][j](p,q).real(),0.5*CPanMat[i][j](p,q).imag()+0.5*CIMat[i][j](p,q).imag());
						CPanMat[i][j](p,q)=	abs(CPanMat[i][j](p,q)) > abs(CIMat[i][j](p,q))? CPanMat[i][j](p,q):CIMat[i][j](p,q);
					}
		//释放内存
		CIMat.clear();

		CpxNumMat Fusion(PAN_Height, PAN_Width); 
		ifdct_wrapping(PAN_Height, PAN_Width, nbscales, nbangles_coarse, ac, CPanMat, Fusion);

		//释放内存
		CPanMat.clear();
		//用融合后的图像替换I分量生成新HCS
		for(i=0; i<PAN_Height; i++)
			for(j=0; j<PAN_Width; j++){

				DATA2D(New_MSData, 0, i*PAN_Width+j, COLS) = Fusion(i,j).real() > 0 ? (float)Fusion(i,j).real():0;// 
			}
		//释放内存
		delete[] Fusion._data;Fusion._data=NULL;//clear(Fusion);

		//将MS从HSI变换到RGB空间
		HCS_Itrans(New_MSData, PAN_Height, PAN_Width, MS_Bandcount);

		//GDAL写文件
		Log(LogName,"01|04");//写入log日志

		if (MSInf->WriteImageInf(Output_MS_FileName)!=0) //创建文件，写入头文件信息
		{
			cerr<<"Write MS Image Inf Error."<<endl;
			cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
			throw 4;
		}

		MSInf->SetImgData(New_MSData);

		if(MSInf->WriteImageFromBuff(Output_MS_FileName,0,0,PAN_Width,PAN_Height)!=0){
			cerr<<"Read PAN Image Data Error."<<endl;
			cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
			throw 4;
		}
		//释放内存
		MSInf->ClearImageData();
		PANData = NULL;    
		MSData = NULL;  
		New_MSData = NULL;

		//释放内存
		delete MSInf;	MSInf = NULL;
		delete PANInf;	PANInf = NULL;

		Log(LogName,"01|05");//写入log日志
	}
	catch(int e){
		if (e == 1)
		{
			Log(LogName,"02|01");//写入log日志
		}
		else if (e == 2)
		{
			Log(LogName,"02|02");//写入log日志
		}
		else if (e == 3)
		{
			Log(LogName,"02|03");//写入log日志
		}
		else if (e == 4)
		{
			Log(LogName,"02|04");//写入log日志
		}
		else{
			//Log(LogName,"02|03");//写入log日志
		}
		return;
	}
	catch(char *str){
		//linux下 fftw malloc.c exit(EXIT_FAILURE);改为throw "fftw error";
		//windows下 会出错
		Log(LogName,"02|03");//写入log日志
		return;
	}

}
