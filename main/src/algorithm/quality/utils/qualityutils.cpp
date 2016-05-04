#include "qualityUtils.h"

//write status into log file
bool WriteMsg(char* logfilepath,int32_t statusnum, char* statusmsg)
{
    FILE *LogFile;
    LogFile=fopen(logfilepath,"wb");
    if (LogFile != NULL) {
        fprintf(LogFile,"%d\n%s\n",statusnum,statusmsg);
        fclose(LogFile);
        return false;
    }
    return true;
}

bool read_ConfigureFile_Parameters(const char* parafilepath, ImageParameter& testparameter) {
    FILE *fp1;
    fp1=fopen(parafilepath,"rb");
    if(fp1 == NULL) {
        printf("Parafile is not exist!\n");
        return false;
    }

    char filename[256];
    int32_t tempnum;

    //read 'filename' from parafile
    if(fscanf(fp1,"%s",&filename)==-1)
    {
        printf("Parafile read error!\n");
        fclose(fp1);
        return false;
    } else {
        cout << filename << endl;
        testparameter.filePath.assign(filename);
    }

    //read 'RowNum' from file
    if(fscanf(fp1,"%d",&tempnum)==-1) {
        printf("Parafile read error!\n");
        fclose(fp1);
        return false;
    } else {
        testparameter.rowNum = tempnum;
    }

    //read 'ColumnNum' from file
    if(fscanf(fp1,"%d",&tempnum) == -1) {
        printf("Parafile read error!\n");
        fclose(fp1);
        return false;
    } else {
        testparameter.columnNum = tempnum;
    }

    //read 'BandNum' from file
    if(fscanf(fp1,"%d",&tempnum) == -1) {
        printf("Parafile read error!\n");
        fclose(fp1);
        return false;
    } else {
        testparameter.bandNum = tempnum;
    }

    //read 'Pixel bits' from file
    if(fscanf(fp1,"%d",&tempnum) == -1) {
        printf("Parafile read error!\n");
        fclose(fp1);
        return false;
    } else {
        testparameter.bitsPerPixel = tempnum;
    }

    fclose(fp1);
    return true;
}

void* qualityInterface() {
    char* parafilepath = "/home/fighter/Documents/Quality/data/para1.para";
    char* logfilepath = "/home/fighter/Documents/Quality/data/log.log";
    int algorithmClass = 1;
    bool flag = false;
    switch (algorithmClass) {
    case 1:
        flag = mainClarity(parafilepath, logfilepath);
        break;
    case 2:
        flag = mainContrastRatio(parafilepath, logfilepath);
        break;
    case 3:
        flag = mainEntropy(parafilepath, logfilepath);
        break;
    case 4:
        flag = mainMean(parafilepath, logfilepath);
        break;
    case 5:
        flag = mainSignaltoNoiseRatio(parafilepath, logfilepath);
        break;
    case 6:
        flag = mainStriperesidual(parafilepath, logfilepath);
        break;
    default:
        break;
    }
    return 0;
}
