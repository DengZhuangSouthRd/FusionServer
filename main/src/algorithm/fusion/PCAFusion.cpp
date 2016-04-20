#include "PCAFusion.h"

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
        int i;
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

