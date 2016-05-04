/*
//Image mean algorithm for geotiff (using GDAL)
//tif图像-均值算法
//Author: bangyong Qin
//Date:2015-9-24
*/

#include "../utils/qualityUtils.h"

//Mean
int32_t Mean(char* filepath,char* logfilepath,double* result) {
	//定义数据集，打开文件
    GDALDataset *poDataset = NULL;
	GDALAllRegister();
	poDataset=(GDALDataset *)GDALOpen(filepath,GA_ReadOnly);
    if( poDataset == NULL ) {
		printf("Image file open error!\n");
		WriteMsg(logfilepath,-1,"Image file open error!");
		return -1; 
    } else {
		printf("Mean algorithm is executing!\n");
		WriteMsg(logfilepath,0,"Mean algorithm is executing!");
	}

	//开始解析图像数据集
	int32_t n,i,j;
	int32_t bandnum,width,height;
	bandnum=poDataset->GetRasterCount(); 
	width=poDataset->GetRasterXSize();
	height=poDataset->GetRasterYSize();
	GDALRasterBand *pband;
	double *meanresult=result;
	uint16_t *banddata;
	banddata=(uint16_t *)CPLMalloc(sizeof(uint16_t)*width*height);

    for(n=0;n<bandnum;n++) {
		pband=poDataset->GetRasterBand(n+1);
		pband->RasterIO(GF_Read,0,0,width,height,banddata,width,height,GDT_UInt16,0,0);
		int64_t count=0;
		uint64_t sum=0;
        for(i=0;i<height;i++) {
            for(j=0;j<width;j++) {
                if(banddata[i*width+j]>0) {
					sum=sum+banddata[i*width+j];
					count++;
				}
			}
		}
		if (count==0) {
			meanresult[n]=0;	
        } else {
			meanresult[n]=sum/(1.0*count);
		}

		GDALClose(pband);
		pband=NULL;

		//Writing the process and status of this Algorithm.
		int32_t temp = (int)(100.0*(n+1)/bandnum);
		temp = (temp>99) ? 99:temp;
		printf("Mean algorithm is executing %d%%!\n",temp);
		WriteMsg(logfilepath,temp,"Mean algorithm is executing!");
    }

	//释放内存，关闭数据集
	CPLFree(banddata);
	banddata=NULL;

	GDALClose(poDataset);
	poDataset=NULL;
	return 1;
}

//主函数
bool mainMean(char* parafilepath, char* logfilepath) {
    ImageParameter testparameter;

    bool flag = read_ConfigureFile_Parameters(parafilepath, testparameter);
    if(flag == false) return false;

    double* result = (double*)malloc(sizeof(double)*testparameter.bandNum);
    if(result == NULL) return false;

    int32_t res;
    res = Mean(const_cast<char*>(testparameter.filePath.c_str()), logfilepath, result);

    if(res != 1) {
        printf("Algorithm executing error!\n");
        WriteMsg(logfilepath,-1,"Algorithm executing error!");
        free(result);
        return false;
    }

    FILE *fp2 = NULL;
    fp2=fopen(logfilepath,"wb");
    for(int i=0;i<testparameter.bandNum;i++) {
        fprintf(fp2,"%.2f,",result[i]);
    }
    fclose(fp2);
    free(result);

    return true;
}
