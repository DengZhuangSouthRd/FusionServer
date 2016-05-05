/*
//Image Striperesidual algorithm for geotiff (using GDAL)
//tif图像-条纹残余量 -改进
//Author: bangyong Qin
//Date:2015-11-17
*/

#include "../utils/qualityUtils.h"

//Striperesidual-条纹残余量 -改进后
int32_t Striperesidual(char* filepath,char* logfilepath,double* result) {
    GDALDataset *poDataset = NULL;
    GDALAllRegister();
    poDataset=(GDALDataset *)GDALOpen(filepath,GA_ReadOnly);
    if( poDataset == NULL ) {
        printf("Image file open error!\n");
        WriteMsg(logfilepath,-1,"Image file open error!");
        return -1;
    } else {
        printf("Striperesidual algorithm is executing!\n");
        WriteMsg(logfilepath,0,"Striperesidual algorithm is executing!");
    }

    //开始解析图像数据集
    int32_t n,i,j;
    int32_t bandnum,width,height;
    bandnum=poDataset->GetRasterCount();
    width=poDataset->GetRasterXSize();
    height=poDataset->GetRasterYSize();
    GDALRasterBand *pband;
    double *striperesult=result;
    uint16_t *banddata=(uint16_t *)CPLMalloc(sizeof(uint16_t)*width*height);

    //设置图像参数
    double maxmark=0.05;
    double minmark=0.001;

    for(n=0;n<bandnum;n++) {
        pband=poDataset->GetRasterBand(n+1);
        pband->RasterIO(GF_Read,0,0,width,height,banddata,width,height,GDT_UInt16,0,0);
        double *colmeans=(double *)CPLMalloc(sizeof(double)*width);

        //数组初始化为0
        for(j=0;j<width;j++)
            colmeans[j]=0.0;

        //依次计算各列的像素均值
        for(i=0;i<height;i++)
            for(j=0;j<width;j++)
                colmeans[j]=colmeans[j]+banddata[i*width+j]/(double)height;

        //计算相邻列像素均值相对差
        int32_t stripemark=0;
        int32_t stripecount=0;
        for(j=1;j<width;j++) {
            double changevalue,changerate;
            changevalue=fabs(colmeans[j]-colmeans[j-1]);
            if(colmeans[j-1]>10.0) {
                changerate=changevalue/colmeans[j-1];
            } else {
                changerate=changevalue/10.0;
            }
            if(changevalue<=minmark) {
                stripemark=1;
                stripecount++;
            } else if(changerate>=maxmark) {
                stripemark=1;
                stripecount++;
            } else if(stripemark==1) {
                stripemark=0;
                stripecount--;
            } else {
                stripemark=0;
            }
        }
        striperesult[n]=100.0*stripecount/width;

        //释放内存，关闭数据集
        CPLFree(colmeans);
        colmeans=NULL;

        GDALClose(pband);
        pband=NULL;

        //Writing the process and status of this Algorithm.
        int32_t temp = (int)(100.0*(n+1)/bandnum);
        temp = (temp>99) ? 99:temp;
        printf("Striperesidual algorithm is executing %d%%!\n",temp);
        WriteMsg(logfilepath,temp,"Striperesidual algorithm is executing!");
    }

    //释放内存，关闭数据集
    CPLFree(banddata);
    banddata=NULL;

    GDALClose(poDataset);
    poDataset=NULL;
    return 1;
}

//主函数
QualityRes mainStriperesidual(char* parafilepath, char* logfilepath) {
    ImageParameter testparameter;
    QualityRes m_qRes;
    m_qRes.status = -1;
    bool flag = read_ConfigureFile_Parameters(parafilepath, testparameter);
    if(flag == false) return m_qRes;

    m_qRes.data = (double*)malloc(sizeof(double)*testparameter.bandNum);
    m_qRes.length = testparameter.bandNum;
    m_qRes.status = -1;
    if(m_qRes.data == NULL) return m_qRes;

    int32_t res;
    res = Striperesidual(const_cast<char*>(testparameter.filePath.c_str()), logfilepath, m_qRes.data);

    if(res != 1) {
        printf("Algorithm executing error!\n");
        WriteMsg(logfilepath,-1,"Algorithm executing error!");
        free(m_qRes.data);
        return m_qRes;
    }

    m_qRes.status = 1;
    return m_qRes;
}