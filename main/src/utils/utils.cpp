#include "utils.h"

void checkFileInfo(string filePath, ResultStatus& status) {
    if(access(filePath.c_str(), 0) == 0) {

        status.stauts = 0;
        status.mode = EXISTS;
        status.desc = "EXISTS";

        if(access(filePath.c_str(), 2) == 0) {
            status.stauts = 0;
            status.mode = WRITEABLE;
            status.desc = "WRITEABLE";
        }

        if(access(filePath.c_str(), 4) == 0) {
            status.stauts = 0;
            status.mode = READABLE;
            status.desc = "READABLE";
        }

        if(access(filePath.c_str(), 6) == 0) {
            status.stauts = 0;
            status.mode = RWABLE;
            status.desc = "RWABLE";
        }
    } else {
        status.stauts = -1;
        status.mode = NOTEXISTS;
        status.desc = "NOTEXISTS";
    }
}

//FusionStruct fusion(string PanUrl,string MsUrl,string OutUrl,string LogUrl,int idalg,int* band,int interpolation);
void* fusionInterface(void * args) {
    FusionArgs* param = (FusionArgs*) args;
    FusionStruct* pObj = (FusionStruct*)malloc(sizeof(FusionStruct));
    int* band = (int*)malloc(sizeof(int) * param->band.size());
    for(size_t i=0;i<param->band.size();++i)
        band[i] = param->band[i];
    *pObj = fusion(param->panurl, param->msurl, param->outurl, param->logurl, param->idalg, band, param->idinter);
    free(band);
    return (void*)pObj;
}
