#include "../utils/qualityutils.h"

int32_t SpectralAngleMatrix(char* filepath2,char* filepath3,char* logfilepath, vector<int> bandlist, vector<double>& SpectralAngleMatrix) {
    GDALDataset *poDataset2,*poDataset3;
    GDALAllRegister();
    poDataset2=(GDALDataset *)GDALOpen(filepath2,GA_ReadOnly);
    poDataset3=(GDALDataset *)GDALOpen(filepath3,GA_ReadOnly);
    if( (poDataset2 == NULL) || (poDataset3 == NULL)  ) {
        WriteMsg(logfilepath,-1,"Image file open error!");
        GDALClose(poDataset2);
        poDataset2=NULL;
        GDALClose(poDataset3);
        poDataset3=NULL;
        return -1;
    } else {
        WriteMsg(logfilepath,0,"SpectralAngleMatrix algorithm is executing!");
    }

    int32_t n,i,j;
    int32_t bandnum2,width2,height2;
    int32_t bandnum3,width3,height3;

    bandnum2=poDataset2->GetRasterCount();
    width2=poDataset2->GetRasterXSize();
    height2=poDataset2->GetRasterYSize();

    bandnum3=poDataset3->GetRasterCount();
    width3=poDataset3->GetRasterXSize();
    height3=poDataset3->GetRasterYSize();

    if(bandlist.size() != bandnum3 || width2 != width3 || height2 != height3) {
        GDALClose(poDataset2);
        poDataset2=NULL;
        GDALClose(poDataset3);
        poDataset3=NULL;
        return -1;
    }

    GDALRasterBand *pband;
    uint16_t *banddata2,*banddata3;
    float *tempdata1,*tempdata2,*tempdata3;
    banddata2=(uint16_t *)CPLMalloc(sizeof(uint16_t)*width2*height2);
    banddata3=(uint16_t *)CPLMalloc(sizeof(uint16_t)*width3*height3);
    tempdata1=(float *)CPLMalloc(sizeof(float)*width3*height3);
    tempdata2=(float *)CPLMalloc(sizeof(float)*width3*height3);
    tempdata3=(float *)CPLMalloc(sizeof(float)*width3*height3);

    for(n=0;n<bandnum3;n++) {
        pband=poDataset2->GetRasterBand(bandlist[n]);
        pband->RasterIO(GF_Read,0,0,width2,height2,banddata2,width2,height2,GDT_UInt16,0,0);
        GDALClose(pband);
        pband=NULL;
        pband=poDataset3->GetRasterBand(n+1);
        pband->RasterIO(GF_Read,0,0,width3,height3,banddata3,width3,height3,GDT_UInt16,0,0);
        GDALClose(pband);
        pband=NULL;

        int32_t tempnum2=0;
        int32_t tempnum3=0;

        for(i=0;i<height3;i++) {
            for(j=0;j<width3;j++) {
                if(n==0) {
                    tempdata1[i*width3+j]=0.0;
                    tempdata2[i*width3+j]=0.0;
                    tempdata3[i*width3+j]=0.0;
                }

                tempnum2=banddata2[i*width2+j];
                tempnum3=banddata3[i*width3+j];
                if(tempnum2>0 && tempnum3>0) {
                    tempdata1[i*width3+j]=tempdata1[i*width3+j]+(1.0*tempnum2*tempnum3/bandnum3);
                    tempdata2[i*width3+j]=tempdata2[i*width3+j]+(1.0*tempnum2*tempnum2/bandnum3);
                    tempdata3[i*width3+j]=tempdata3[i*width3+j]+(1.0*tempnum3*tempnum3/bandnum3);
                }
            }
        }

        int32_t temp = (int)(100.0*(n+1)/bandnum3);
        temp = (temp>99) ? 99:temp;
        WriteMsg(logfilepath,temp,"SpectralAngleMatrix algorithm is executing!");
    }

    CPLFree(banddata2);
    banddata2=NULL;
    CPLFree(banddata3);
    banddata3=NULL;

    int64_t count=0;
    long double sum=0.0;
    for(i=0;i<height3;i++) {
        for(j=0;j<width3;j++) {
            if(j==20) {
                int a=2;
            }
            if(tempdata2[i*width3+j]!=0 && tempdata3[i*width3+j]!=0) {
                double tempdbl1=((double)tempdata1[i*width3+j])/(sqrt(tempdata2[i*width3+j])*sqrt(tempdata3[i*width3+j]));
                if(tempdbl1>1.0) {
                    tempdbl1=1.0;
                } else if(tempdbl1<-1.0) {
                    tempdbl1=-1.0;
                }
                sum=sum+acos(tempdbl1);
                count++;
            }
        }
    }
    sum = sum/count;
    SpectralAngleMatrix.push_back(sum);

    CPLFree(tempdata1);
    tempdata1=NULL;
    CPLFree(tempdata2);
    tempdata2=NULL;
    CPLFree(tempdata3);
    tempdata3=NULL;

    GDALClose(poDataset2);
    poDataset2=NULL;
    GDALClose(poDataset3);
    poDataset3=NULL;
    return 1;
}

bool mainSpectralAngleMatrix(string filepath2, string filepath3, char* logfile, vector<int> bandlist, vector<double>& m_qRes) {
    m_qRes.clear();
    int32_t res = SpectralAngleMatrix(const_cast<char*>(filepath2.c_str()), const_cast<char*>(filepath3.c_str()), logfile, bandlist, m_qRes);
    if(res != 1) {
        WriteMsg(logfile,-1,"Algorithm mainSpectralAngleMatrix executing error!");
        return false;
    }
    return true;
}