/********************************************************************
	created:	2015/12/24
	created:	24:12:2015   14:33
	filename: 	D:\code\vs2010\C\Fusion\PCAFusion.cpp
	file path:	D:\code\vs2010\C\Fusion
	file base:	PCAFusion
	file ext:	cpp
	author:		YS
	
	purpose:	PCAFusion类实现
				主成分分析（PCA）融合算法
*********************************************************************/
#include "PCAFusion.h"

void PCAFusion::Local_MaxMin_PCA_Fusion(const char* Input_PAN_FileName, const char* Input_MS_FileName, const char* Output_MS_FileName,int InterpolationMethod){
	/*
	 *融合方法：PCA
	 *融合流程：对低分图像插值->对低分PCA变换->对高分灰度拉伸->替换第一主成分->PCA反变换
	 *替换规则：第一主成分成分替换
	 *Input_PAN_FileName  高分辨率图像路径及名称
	 *Input_MS_FileName   多光谱图像路径及名称
	 *Output_MS_FileName  融合图像路径及名称
	 *InterpolationMethod       插值方法
	 *作者：YS
	 */

	//修改内容：因为第一主成分含有负值，不适合直方图匹配，
	//			融合时采用将Pan图像的灰度范围拉伸到第一主成分的灰度范围  
	//			(PAN-PAN_MIN)/(PAN_MAX-PAN_MIN) *(MS_MAX-MS_MIN)+MS_MIN
	//待改进：（插值、变换）没有考虑节省内存，错误判断和提示
	//已改进：（1）变换考虑节省内存
	//		  （2）错误判断和提示
	//作者：YS
	//修改时间：2015.11.12

	//存在的问题：结果分块
	//解决办法：
	//作者：YS
	//日期：2015.12.24
	GDALAllRegister();         //利用GDAL读取图片，先要进行注册  
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");   //设置支持中文路径 

	int i,j;

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
		cerr<<"PCA Fusion Error."<<endl;
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

	float* MS_Avg = new float[MS_Bandcount];
	float* MS_EigenVector = new float[MS_Bandcount*MS_Bandcount];
	float* MS_EigenValue = new float[MS_Bandcount];
	if (NULL == MS_Avg||NULL == MS_EigenVector||NULL == MS_EigenValue)//
	{
		cerr<<"Memory Error.\n";
		cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
		exit(1);
	}

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
			MSInf->ClearImageData(); MSData = NULL;

			//读取PAN图像数据
			if (PANInf->ReadImageToBuff(Input_PAN_FileName,x*ratio,y*ratio,width*ratio,height*ratio) != 0) 
			{
				cerr<<"Read PAN Image Data Error.\n";
				cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
				exit(1);
			}
			PANData = PANInf->GetImgData();    //PAN图像数据指针


			int ROWS = MS_Bandcount;
			int COLS = height*ratio*width*ratio;


			//对MS做PCA变换

			//对插值后的MS图像做PCA变换
			PCAProject(New_MSData, ROWS, COLS, MS_Avg, MS_EigenVector, MS_EigenValue);
			int max_eigen=0;
			for(i=1;i<ROWS;i++){
				//cout<<MS_EigenValue[i]<<endl;
				if (abs(MS_EigenValue[i])>abs(MS_EigenValue[max_eigen]))
				{
					max_eigen=i;
				}
			}

			//将Pan图像的灰度范围拉伸到第一主成分的灰度范围  (PAN-PAN_MIN)/(PAN_MAX-PAN_MIN) *(MS_MAX-MS_MIN)+MS_MIN
	
			float max,min;

			MaxMin(PANData,height*ratio,width*ratio,max,min);
			for (i = 0; i < COLS; i++)
				PANData[i] = (PANData[i]-min)/(max-min);
			cout<<"最大值："<<max<<" 最小值："<<min<<endl;
			MaxMin(New_MSData+max_eigen*COLS,height*ratio,width*ratio,max,min);
			for (i = 0; i < COLS; i++)
				PANData[i] = PANData[i]*(max-min)+min;
			cout<<"最大值："<<max<<" 最小值："<<min<<endl;

			//进行PAN影像与MS影像融合
			for (i = 0; i < COLS; i++)
				DATA2D(New_MSData, max_eigen, i, COLS) = PANData[i];

			//释放内存
			PANInf->ClearImageData();PANData = NULL;

			//PCA反变换
			PCAReproject(New_MSData, ROWS, COLS, MS_Avg, MS_EigenVector);
			


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
	delete[] MS_Avg; MS_Avg = NULL;
	delete[] MS_EigenVector; MS_EigenVector = NULL;
	delete[] MS_EigenValue; MS_EigenValue = NULL;

	delete MSInf;	MSInf=NULL;
	delete PANInf;	PANInf=NULL;
}

void PCAFusion::Local_MeanStd_PCA_Fusion(const char* Input_PAN_FileName, const char* Input_MS_FileName, const char* Output_MS_FileName,int InterpolationMethod){
	/*
	 *融合方法：PCA
	 *融合流程：对低分图像插值->对低分PCA变换->对高分灰度拉伸->替换第一主成分->PCA反变换
	 *替换规则：第一主成分成分替换
	 *Input_PAN_FileName  高分辨率图像路径及名称
	 *Input_MS_FileName   多光谱图像路径及名称
	 *Output_MS_FileName  融合图像路径及名称
	 *InterpolationMethod       插值方法
	 *作者：YS
	 */

	//修改内容：因为第一主成分含有负值，不适合直方图匹配，
	//			融合时采用将Pan图像的灰度范围拉伸
	//			(PAN-PAN_Mean)/(PAN_Var) *(MS_Var)+MS_Mean
	//待改进：（插值、变换）没有考虑节省内存，错误判断和提示
	//已改进：（1）变换考虑节省内存
	//		  （2）错误判断和提示
	//作者：YS
	//修改时间：2015.11.12

	//存在的问题：结果分块
	//解决办法：
	//作者：YS
	//日期：2015.12.24
	GDALAllRegister();         //利用GDAL读取图片，先要进行注册  
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");   //设置支持中文路径 

	int i,j;

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

	float* MS_Avg = new float[MS_Bandcount];
	float* MS_EigenVector = new float[MS_Bandcount*MS_Bandcount];
	float* MS_EigenValue = new float[MS_Bandcount];
	if (NULL == MS_Avg||NULL == MS_EigenVector||NULL == MS_EigenValue)//
	{
		cerr<<"Memory Error.\n";
		cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
		exit(1);
	}

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
			MSInf->ClearImageData(); MSData = NULL;

			//读取PAN图像数据
			if (PANInf->ReadImageToBuff(Input_PAN_FileName,x*ratio,y*ratio,width*ratio,height*ratio) != 0) 
			{
				cerr<<"Read PAN Image Data Error.\n";
				cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
				exit(1);
			}
			PANData = PANInf->GetImgData();    //PAN图像数据指针


			int ROWS = MS_Bandcount;
			int COLS = height*ratio*width*ratio;


			//对MS做PCA变换

			//对插值后的MS图像做PCA变换
			PCAProject(New_MSData, ROWS, COLS, MS_Avg, MS_EigenVector, MS_EigenValue);
			int max_eigen=0;
			for(i=1;i<ROWS;i++){
				//cout<<MS_EigenValue[i]<<endl;
				if (abs(MS_EigenValue[i])>abs(MS_EigenValue[max_eigen]))
				{
					max_eigen=i;
				}
			}

			//将Pan图像的灰度范围拉伸  (MS_Var)*(PAN-PAN_Mean)/(PAN_Var) +MS_Mean
	
			float mean,s;//均值 标准差

			MeanStd(PANData,height*ratio,width*ratio,s,mean);
			for (i = 0; i < COLS; i++)
				PANData[i] = (PANData[i]-mean)/(s);
			
			MeanStd(New_MSData+max_eigen*COLS,height*ratio,width*ratio,s,mean);
			for (i = 0; i < COLS; i++)
				PANData[i] = PANData[i]*(s)+mean;


			//进行PAN影像与MS影像融合
			for (i = 0; i < COLS; i++)
				DATA2D(New_MSData, max_eigen, i, COLS) = PANData[i];

			//释放内存
			PANInf->ClearImageData();PANData = NULL;

			//PCA反变换
			PCAReproject(New_MSData, ROWS, COLS, MS_Avg, MS_EigenVector);
			


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
	delete[] MS_Avg; MS_Avg = NULL;
	delete[] MS_EigenVector; MS_EigenVector = NULL;
	delete[] MS_EigenValue; MS_EigenValue = NULL;

	delete MSInf;	MSInf=NULL;
	delete PANInf;	PANInf=NULL;
}


void PCAFusion::Global_MaxMin_PCA_Fusion(const char* Input_PAN_FileName, const char* Input_MS_FileName, const char* Output_MS_FileName,int InterpolationMethod){
	/*
	 *融合方法：PCA
	 *融合流程：对低分图像插值->对低分PCA变换->对高分灰度拉伸->替换第一主成分->PCA反变换
	 *替换规则：第一主成分成分替换
	 *Input_PAN_FileName  高分辨率图像路径及名称
	 *Input_MS_FileName   多光谱图像路径及名称
	 *Output_MS_FileName  融合图像路径及名称
	 *InterpolationMethod       插值方法
	 *作者：YS
	 */

	//修改内容：因为第一主成分含有负值，不适合直方图匹配，
	//			融合时采用将Pan图像的灰度范围拉伸到第一主成分的灰度范围  
	//			(PAN-PAN_MIN)/(PAN_MAX-PAN_MIN) *(MS_MAX-MS_MIN)+MS_MIN
	//待改进：（插值、变换）没有考虑节省内存，错误判断和提示
	//已改进：（1）变换考虑节省内存
	//		  （2）错误判断和提示
	//作者：YS
	//修改时间：2015.11.12

	//存在的问题：结果分块
	//解决办法：全局处理
	//作者：YS
	//日期：2015.12.30

	//修改内容：在数据本身做PCA正反变换，节省内存
	//作者：YS
	//日期：2015.12.31
	GDALAllRegister();         //利用GDAL读取图片，先要进行注册  
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");   //设置支持中文路径 

	int i,j,k;

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
	float max,min;           //局部极值
	float panmax,panmin,pmax,pmin;//全局极值
	panmax = pmax = -100000;  //全色图像最大值和第一主成分最大值
	panmin = pmin =100000;	  //全色图像最小值和第一主成分最小值
	float* MS_Avg = new float[MS_Bandcount];
	float* MS_EigenVector = new float[MS_Bandcount*MS_Bandcount];
	float* MS_EigenValue = new float[MS_Bandcount];
	float* covariance = new float[MS_Bandcount*MS_Bandcount];
	float* tmp = new float[MS_Bandcount];
	//float* MS_ProjectPCA = NULL;
	if (NULL == MS_Avg||NULL == MS_EigenVector||NULL == MS_EigenValue||NULL == covariance||NULL == tmp)//
	{
		cerr<<"Memory Error.\n";
		cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
		exit(1);
	}
	//初始化
	for (i=0;i<MS_Bandcount;i++)
		MS_Avg[i]=0;
	for (i=0;i<MS_Bandcount*MS_Bandcount;i++)
		covariance[i]=0;
	//求全局各波段均值
	cout<<"计算各波段均值。"<<endl;
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


			if (MSInf->ReadImageToBuff(Input_MS_FileName,x,y,width,height) != 0)//读取MS图像数据
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

			float tmp;
			//求全局各波段均值
			for (i=0;i<MS_Bandcount;i++){
				tmp = 0;
				for (j=0;j<COLS;j++)
				{
					tmp += DATA2D(New_MSData, i, j, COLS);
					
				}
				tmp = tmp/(PAN_Height*PAN_Width);
				MS_Avg[i] += tmp;
			}

			//释放内存
			delete[] New_MSData;New_MSData = NULL;

			x=n*width;	
		}
		y=m*height;
		x=0;
	}

	//求协方差矩阵
	cout<<"计算协方差矩阵。"<<endl;
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

			if (MSInf->ReadImageToBuff(Input_MS_FileName,x,y,width,height) != 0)//读取MS图像数据
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
			MSInf->ClearImageData(); MSData = NULL;

			int ROWS = MS_Bandcount;
			int COLS = height*ratio*width*ratio;

			float tmp;
			//求协方差矩阵
			for (i = 0; i < ROWS; i++) /* subtract mean */
				for (j = 0; j < COLS; j++)
				{
					New_MSData[i*COLS + j] = New_MSData[i*COLS + j] - MS_Avg[i];
				}
			
			for (i = 0; i < ROWS; i++){
				for (j = 0; j < ROWS; j++)
				{
					tmp = 0;
					for (k = 0; k < COLS; k++)
						tmp += New_MSData[i*COLS + k] * New_MSData[j*COLS + k];
					tmp = tmp / (PAN_Height*PAN_Width - 1);
					covariance[j*ROWS + i] += tmp;
					//cout<<tmp<<" ";
				}
				//cout<<endl;
			}

			//释放内存
			delete[] New_MSData;New_MSData = NULL;

			x=n*width;	
		}
		y=m*height;
		x=0;
	}
	cout<<"计算特征值和特征向量。"<<endl;
	/* find the eigenvectors and eigenvalues */
	int rvalue = SymmetricRealMatrix_Eigen(covariance, MS_Bandcount, MS_EigenValue, MS_EigenVector);
	delete[] covariance;covariance = NULL;
	if (rvalue<0)
	{
		cerr<<"PCA：Find the eigenvectors and eigenvalues error.\n";
		cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
		exit(1);
	}

	int max_eigen = 0;
	for(i=0;i<MS_Bandcount;i++){
		cout<<MS_EigenValue[i]<<endl;
		if (abs(MS_EigenValue[i])>abs(MS_EigenValue[max_eigen]))
		{
			max_eigen=i;
		}
	}
	//cout<<max_eigen<<endl;

	//求第一主成分和全色图像极值
	cout<<"计算第一主成分和全色图像极值。"<<endl;
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

			if (MSInf->ReadImageToBuff(Input_MS_FileName,x,y,width,height) != 0)//读取MS图像数据
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
			MSInf->ClearImageData(); MSData = NULL;

			int ROWS = MS_Bandcount;
			int COLS = height*ratio*width*ratio;

			
			//求第一主成分
			float temp;
			for (i = 0; i < COLS; i++){
				temp = 0.0;
				for (k = 0; k < ROWS; k++)
					temp += MS_EigenVector[k*ROWS + max_eigen] * (New_MSData[k*COLS + i]-MS_Avg[k]);

				New_MSData[max_eigen*COLS + i] = temp;
			}

			//求极值
			MaxMin(New_MSData+max_eigen*COLS,height*ratio,width*ratio,max,min); 
			if (pmax < max) pmax = max;
			if (pmin > min) pmin = min;	

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
	cout<<"全色图像的最大值是："<<panmax <<" 最小值是："<<panmin<<endl;
	cout<<"第一主成分的最大值是："<<pmax <<" 最小值是："<<pmin<<endl;
	cout<<"开始融合。"<<endl;
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
			MSInf->ClearImageData(); MSData = NULL;

			//读取PAN图像数据
			if (PANInf->ReadImageToBuff(Input_PAN_FileName,x*ratio,y*ratio,width*ratio,height*ratio) != 0) 
			{
				cerr<<"Read PAN Image Data Error.\n";
				cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
				exit(1);
			}
			PANData = PANInf->GetImgData();    //PAN图像数据指针

			int ROWS = MS_Bandcount;
			int COLS = height*ratio*width*ratio;


			//对MS做PCA变换

			//MS_ProjectPCA = new float[ROWS*COLS];
			//if (NULL == MS_ProjectPCA)//
			//{
			//	cerr<<"Memory Error.\n";
			//	cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
			//	exit(1);
			//}
			////对插值后的MS图像做PCA变换
			//
			//for (i = 0; i < ROWS; i++)
			//	for (j = 0; j < COLS; j++)
			//	{
			//		MS_ProjectPCA[i*COLS + j] = 0.0;
			//		for (k = 0; k < ROWS; k++)
			//			MS_ProjectPCA[i*COLS + j] += MS_EigenVector[k*ROWS + i] *(New_MSData[k*COLS + j]-MS_Avg[k]);
			//	}

			for (i = 0; i < COLS; i++){
				for (j = 0; j < ROWS; j++)
				{
					tmp[j] = 0.0;
					for (k = 0; k < ROWS; k++)
						tmp[j] += MS_EigenVector[k*ROWS + j] * (New_MSData[k*COLS + i]-MS_Avg[k]);
				}
				for (j = 0; j < ROWS; j++)
				{
					New_MSData[j*COLS + i] = tmp[j];
				}
			}

			//将Pan图像的灰度范围拉伸到第一主成分的灰度范围  (PAN-PAN_MIN)/(PAN_MAX-PAN_MIN) *(MS_MAX-MS_MIN)+MS_MIN
	
			for (i = 0; i < COLS; i++)
				PANData[i] = (PANData[i]-panmin)/(panmax-panmin);
			
			for (i = 0; i < COLS; i++)
				PANData[i] = PANData[i]*(pmax-pmin)+pmin;


			//进行PAN影像与MS影像融合
			for (i = 0; i < COLS; i++)
				DATA2D(New_MSData, max_eigen, i, COLS) = PANData[i];
			
			//释放内存
			PANInf->ClearImageData();PANData = NULL;

			//PCA反变换

			//for (i = 0; i < ROWS; i++)
			//	for (j = 0; j < COLS; j++)
			//	{
			//		New_MSData[i*COLS + j] = 0.0;
			//		for (k = 0; k < ROWS; k++)
			//			New_MSData[i*COLS + j] += MS_EigenVector[i*ROWS + k] *MS_ProjectPCA[k*COLS + j];

			//		New_MSData[i*COLS + j]=New_MSData[i*COLS + j] + MS_Avg[i];
			//	}

			for (i = 0; i < COLS; i++){
				for (j = 0; j < ROWS; j++)
				{
					tmp[j] = 0;
					for (k = 0; k < ROWS; k++)
						tmp[j] += MS_EigenVector[j*ROWS + k] * New_MSData[k*COLS + i];

					tmp[j] = tmp[j] + MS_Avg[j];

				}
				for (j = 0; j < ROWS; j++)
				{
					New_MSData[j*COLS + i] = tmp[j];
					//处理小于0的点
					if (New_MSData[j*COLS + i] < 0)
					{
						New_MSData[j*COLS + i] = 0;
					}
				}

			}

			//释放内存
			//delete[] MS_ProjectPCA; MS_ProjectPCA = NULL;

			MSInf->SetImgData(New_MSData);
			//GDAL写文件
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
	delete[] tmp;tmp = NULL;
	delete[] MS_Avg; MS_Avg = NULL;
	delete[] MS_EigenVector; MS_EigenVector = NULL;
	delete[] MS_EigenValue; MS_EigenValue = NULL;
	delete MSInf;	MSInf = NULL;
	delete PANInf;	PANInf = NULL;
}

void PCAFusion::MeanStd_PCA_Fusion(const char* Input_PAN_FileName, const char* Input_MS_FileName, const char* Output_MS_FileName,const char* LogName,int* bandlist,int InterpolationMethod){
	/*
	 *融合方法：PCA
	 *融合流程：对低分图像插值->对低分PCA变换->对高分灰度拉伸->替换第一主成分->PCA反变换
	 *替换规则：第一主成分成分替换
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
		int i,j;
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


		float* PANData =NULL;    //记录PAN数据
		float* MSData = NULL;    //记录MS原始数据
		float* New_MSData = NULL;  //记录MS插值后数据


		if (MSInf->ReadImageToBuff(Input_MS_FileName,0,0,MS_Width,MS_Height,bandlist) != 0)//读取MS图像数据
		{
			cerr<<"Read MS Image Data Error.\n";
			cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
			throw 1;
		}

		Log(LogName,"01|02");//写入log日志
		//MS图像数据指针
		MSData = MSInf->GetImgData();	  //MS图像数据指针
		New_MSData = new float[PAN_Height*PAN_Width*MS_Bandcount];  //保存MS插值后的影像
		
		if (NULL == New_MSData)//
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
			cerr<<"Read PAN Image Data Error.\n";
			cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
			throw 1;
		}
		PANData = PANInf->GetImgData();    //PAN图像数据指针

		Log(LogName,"01|03");//写入log日志

		int ROWS = MS_Bandcount;
		int COLS = PAN_Width*PAN_Height;

		//对MS做PCA变换
		float* MS_Avg = new float[MS_Bandcount];
		float* MS_EigenVector = new float[MS_Bandcount*MS_Bandcount];
		float* MS_EigenValue = new float[MS_Bandcount];
		if (NULL == MS_Avg||NULL == MS_EigenVector||NULL == MS_EigenValue)//
		{
			cerr<<"Memory Error.\n";
			cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
			throw 3;
		}

		//对插值后的MS图像做PCA变换
		PCAProject(New_MSData, ROWS, COLS, MS_Avg, MS_EigenVector, MS_EigenValue);
		int max_eigen=0;
		for(i=1;i<ROWS;i++){
			//cout<<MS_EigenValue[i]<<endl;
			if (abs(MS_EigenValue[i])>abs(MS_EigenValue[max_eigen]))
			{
				max_eigen=i;
			}
		}

		//将Pan图像的灰度范围拉伸  (MS_Var)*(PAN-PAN_Mean)/(PAN_Var) +MS_Mean
	
		float mean,s;//均值 标准差

		MeanStd(PANData,PAN_Height, PAN_Width,s,mean);
		for (i = 0; i < COLS; i++)
			PANData[i] = (PANData[i]-mean)/(s);
		
		MeanStd(New_MSData+max_eigen*COLS,PAN_Height, PAN_Width,s,mean);
		for (i = 0; i < COLS; i++)
			PANData[i] = PANData[i]*(s)+mean;


		//进行PAN影像与MS影像融合
		for (i = 0; i < COLS; i++)
			DATA2D(New_MSData, max_eigen, i, COLS) = PANData[i];

		//释放内存
		PANInf->ClearImageData();PANData = NULL;

		//PCA反变换
		PCAReproject(New_MSData, ROWS, COLS, MS_Avg, MS_EigenVector);
		

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
			cerr<<"Write MS Image Data Error."<<endl;
			cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
			throw 4;
		}
		//释放内存
		MSInf->ClearImageData();New_MSData = NULL;	

		//释放内存
		delete[] MS_Avg; MS_Avg = NULL;
		delete[] MS_EigenVector; MS_EigenVector = NULL;
		delete[] MS_EigenValue; MS_EigenValue = NULL;

		delete MSInf;	MSInf=NULL;
		delete PANInf;	PANInf=NULL;

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

