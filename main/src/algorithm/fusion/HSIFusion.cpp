/********************************************************************
	created:	2015/12/18
	created:	18:12:2015   9:16
	filename: 	D:\code\vs2010\C\Fusion\HSIFusion.h
	file path:	D:\code\vs2010\C\Fusion
	file base:	HSIFusion
	file ext:	h
	author:		YS
	
	purpose:	HSIFusion类实现
				HSI融合算法
				（1）实现了分块处理
				（2）仅适用于三个波段，对波段的读取顺序有要求，R、G、B
*********************************************************************/

#include "HSIFusion.h"

//通过HIS算法实现PAN影像与MS影像融合
void HSIFusion::Local_MaxMin_HSI_Fusion(const char* Input_PAN_FileName, const char* Input_MS_FileName, const char* Output_MS_FileName,int InterpolationMethod){
	/*
	 *融合方法：HSI
	 *融合流程：对低分图像插值->对低分HSI变换->替换I分量->HSI反变换
	 *替换规则：I分量替换
	 *Input_PAN_FileName		高分辨率图像路径及名称
	 *Input_MS_FileName			多光谱图像路径及名称（必须为三个波段）
	 *Output_MS_FileName		融合图像路径及名称
	 *InterpolationMethod       插值方法
	 *作者：YS
	 */
	
	//待改进：（插值、变换）没有考虑节省内存，错误判断和提示
	//已改进：（1）Brovey变换考虑节省内存；
	//        （2）错误判断和提示
	//作者：YS
	//日期：2015.11.12

	//存在的问题：分块处理时，局部灰度拉伸导致结果分块严重
	//解决办法：去掉“灰度拉伸”步骤，先求全局最大值最小值 再分块处理？
	//作者：YS
	//日期：2015.12.18

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
		cerr<<"HSI Fusion Error."<<endl;
		cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
		exit(1);
	}
	else if(MS_Bandcount > 3)  //如果波段数大于3 取前3个波段
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
	float* PANData = NULL;    //记录PAN数据
	float* MSData = NULL;    //记录MS原始数据
	float* New_MSData = NULL;  //记录MS插值后数据
	
	int count = 1;
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
			MSInf->ClearImageData();

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


			//PAN影像与MS影像融合：用灰度拉伸后的图像替换I分量生成新HSI
			for (i = 0; i < COLS; i++)
				DATA2D(New_MSData, 2, i, COLS) = PANData[i] ; //不需要把PAN图像转换到[0,1]
			
			//释放内存
			PANInf->ClearImageData();

			//将MS从HSI变换到RGB空间
			HSI2RGB(New_MSData, height*ratio,width*ratio, MS_Bandcount);

			//GDAL写文件
			MSInf->SetImgData(New_MSData);
		
			if(MSInf->WriteImageFromBuff(Output_MS_FileName,x*ratio,y*ratio,width*ratio,height*ratio)!=0){
				cerr<<"Read PAN Image Data Error."<<endl;
				cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
				exit(1);
			}
			//释放内存
			MSInf->ClearImageData();
			PANData = NULL;    
			MSData = NULL;  
			New_MSData = NULL;

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

void HSIFusion::Local_MeanStd_HSI_Fusion(const char* Input_PAN_FileName, const char* Input_MS_FileName, const char* Output_MS_FileName,int InterpolationMethod){
	/*
	 *融合方法：HSI
	 *融合流程：对低分图像插值->对低分HSI变换->替换I分量->HSI反变换
	 *替换规则：I分量替换
	 *Input_PAN_FileName		高分辨率图像路径及名称
	 *Input_MS_FileName			多光谱图像路径及名称（必须为三个波段）
	 *Output_MS_FileName		融合图像路径及名称
	 *InterpolationMethod       插值方法
	 *作者：YS
	 */
	
	//待改进：（插值、变换）没有考虑节省内存，错误判断和提示
	//已改进：（1）Brovey变换考虑节省内存；
	//        （2）错误判断和提示
	//作者：YS
	//日期：2015.11.12

	//存在的问题：分块处理时，局部灰度拉伸导致结果分块严重
	//解决办法：求全局均值和方差 再分块处理？
	//作者：YS
	//日期：2015.12.18

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
		cerr<<"HSI Fusion Error."<<endl;
		cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
		exit(1);
	}
	else if(MS_Bandcount > 3)  //如果波段数大于3 取前3个波段
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
	float* PANData = NULL;    //记录PAN数据
	float* MSData = NULL;    //记录MS原始数据
	float* New_MSData = NULL;  //记录MS插值后数据
	
	int count = 1;
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
			MSInf->ClearImageData();

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
			

			float mean,s;//均值 标准差

			MeanStd(PANData,height*ratio,width*ratio,s,mean);

			for (i = 0; i < COLS; i++)
				PANData[i] = (PANData[i]-mean)/(s);

			MeanStd(New_MSData+2*COLS,height*ratio,width*ratio,s,mean); 

			for (i = 0; i < COLS; i++)
				PANData[i] = PANData[i]*(s)+mean;

			//PAN影像与MS影像融合：用灰度拉伸后的图像替换I分量生成新HSI
			for (i = 0; i < COLS; i++)
				DATA2D(New_MSData, 2, i, COLS) = PANData[i] ; //不需要把PAN图像转换到[0,1]
			
			//释放内存
			PANInf->ClearImageData();

			//将MS从HSI变换到RGB空间
			HSI2RGB(New_MSData, height*ratio,width*ratio, MS_Bandcount);

			//GDAL写文件
			MSInf->SetImgData(New_MSData);
		
			if(MSInf->WriteImageFromBuff(Output_MS_FileName,x*ratio,y*ratio,width*ratio,height*ratio)!=0){
				cerr<<"Read PAN Image Data Error."<<endl;
				cerr<<"file："<<__FILE__<<"line："<<__LINE__<<"time："<<__DATE__<<" "<<__TIME__<<endl;
				exit(1);
			}
			//释放内存
			MSInf->ClearImageData();
			PANData = NULL;    
			MSData = NULL;  
			New_MSData = NULL;

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

void HSIFusion::Global_MaxMin_HSI_Fusion(const char* Input_PAN_FileName, const char* Input_MS_FileName, const char* Output_MS_FileName,int InterpolationMethod){
	/*
	 *融合方法：HSI
	 *融合流程：对低分图像插值->对低分HSI变换->替换I分量->HSI反变换
	 *替换规则：I分量替换
	 *Input_PAN_FileName		高分辨率图像路径及名称
	 *Input_MS_FileName			多光谱图像路径及名称（必须为三个波段）
	 *Output_MS_FileName		融合图像路径及名称
	 *InterpolationMethod       插值方法
	 *作者：YS
	 */
	
	//待改进：（插值、变换）没有考虑节省内存，错误判断和提示
	//已改进：（1）Brovey变换考虑节省内存；
	//        （2）错误判断和提示
	//作者：YS
	//日期：2015.11.12

	//存在的问题：分块处理时，局部灰度拉伸导致结果分块严重
	//解决办法：去掉“灰度拉伸”步骤，先求全局最大值最小值 再分块处理？
	//作者：YS
	//日期：2015.12.18

	//存在的问题：分块处理时，局部灰度拉伸导致结果分块严重
	//解决办法：灰度拉伸考虑全局最大值最小值 再分块处理
	//作者：YS
	//日期：2015.12.28

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

			//PAN影像与MS影像融合：用灰度拉伸后的图像替换I分量生成新HSI
			for (i = 0; i < COLS; i++)
				DATA2D(New_MSData, 2, i, COLS) = PANData[i] ; //不需要把PAN图像转换到[0,1]
			
			//释放内存
			PANInf->ClearImageData();PANData = NULL;  

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

void HSIFusion::Global_MeanStd_HSI_Fusion(const char* Input_PAN_FileName, const char* Input_MS_FileName, const char* Output_MS_FileName,int InterpolationMethod){
	/*
	 *融合方法：HSI
	 *融合流程：对低分图像插值->对低分HSI变换->替换I分量->HSI反变换
	 *替换规则：I分量替换
	 *Input_PAN_FileName		高分辨率图像路径及名称
	 *Input_MS_FileName			多光谱图像路径及名称（必须为三个波段）
	 *Output_MS_FileName		融合图像路径及名称
	 *InterpolationMethod       插值方法
	 *作者：YS
	 *日期：2016.01.21
	 */
	
	//待改进：（插值、变换）没有考虑节省内存，错误判断和提示
	//已改进：（1）Brovey变换考虑节省内存；
	//        （2）错误判断和提示
	//作者：YS
	//日期：2015.11.12


	//存在的问题：分块处理时，局部灰度拉伸导致结果分块严重
	//解决办法：灰度拉伸考虑全局均值和方差 再分块处理
	//作者：YS
	//日期：2015.12.28

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
			
			//释放内存
			PANInf->ClearImageData();PANData = NULL;  

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

void HSIFusion::MeanStd_HSI_Fusion(const char* Input_PAN_FileName, const char* Input_MS_FileName, const char* Output_MS_FileName,const char* LogName,int* bandlist,int InterpolationMethod){
	/*
	 *融合方法：HSI
	 *融合流程：对低分图像插值->对低分HSI变换->替换I分量->HSI反变换
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
			PANData[i] = PANData[i]*(s)+mean;

		//PAN影像与MS影像融合：用灰度拉伸后的图像替换I分量生成新HSI
		for (i = 0; i < COLS; i++)
			DATA2D(New_MSData, 2, i, COLS) = PANData[i] > 0 ? PANData[i] : 0; //不需要把PAN图像转换到[0,1]


		//释放内存
		PANInf->ClearImageData();

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
		Log(LogName,"02|03");//写入log日志
		return;
	}

}
