#include "imagequality.h"

ImageQuality::ImageQuality() {
    p_threadPool = NULL;
    p_threadPool = ThreadPool::getSingleInstance();
}

ImageQuality::~ImageQuality() {
    p_threadPool->revokeSingleInstance();
    p_threadPool = NULL;
}

bool ImageQuality::checkQualityArgv(const QualityInputStruct &inputArgs) {
    // main to change the filePath is Right !
    for(QualityMapArgs::const_iterator it=inputArgs.inputMap.begin(); it!=inputArgs.inputMap.end(); it++) {
        if(!(it->second.bitsPerPixel == 8 || it->second.bitsPerPixel == 16 || it->second.bitsPerPixel == 24)) {
            Log::Error("BitsPerPixel Value not in 8, 16, 24 ! Please Check !");
            return false;
        }
        if(it->second.bandNum <= 0) {
            Log::Error("BandNum Value <= 0 ! Please Check !");
            return false;
        }
        if(it->second.colNum <= 0 || it->second.rowNum <= 0) {
            Log::Error("ColNum <=0 Or RowNum <= 0 !");
            return false;
        }
        if(access(it->second.filePath.c_str(), 0) != 0) {
            Log::Error("FilePath Not Exists ! Please Check !");
            return false;
        }
    }
    return true;
}

void ImageQuality::log_InputParameters(const QualityInputStruct &inputArgs) {
    string str = "";
    str += ("task_id="+inputArgs.id+"#");
    str += ("algorithmkind="+to_string(inputArgs.algorithmkind)+"#");
    for(QualityMapArgs::const_iterator it=inputArgs.inputMap.begin(); it!=inputArgs.inputMap.end(); it++) {
        str += (it->first+"$"+"filePath="+it->second.filePath+"#rowNum="+to_string(it->second.rowNum)+"#colNum="+to_string(it->second.colNum));
        str += ("#bandNum="+to_string(it->second.bandNum));
        str += ("#bitsPerPixel="+to_string(it->second.bitsPerPixel));
    }
    Log::Info(str);
}

void ImageQuality::log_OutputResult(const QualityInfo &outRes) {
    string str = "";
    str += ("status="+to_string(outRes.stauts)+"#");
    for(DatasMap::const_iterator it=outRes.imgsquality.begin(); it!=outRes.imgsquality.end(); it++) {
        str += ("key="+it->first+"#");
        str += "bandValue=";
        for(DataArray::const_iterator vit=it->second.begin(); vit!= it->second.end(); vit++) {
            str += (to_string(*vit)+",");
        }
    }
    Log::Info(str);
}

QualityInfo ImageQuality::qualitySyn(const QualityInputStruct &inputArgs, const Ice::Current &) {

}

int ImageQuality::qualityAsyn(const QualityInputStruct &inputArgs, const Ice::Current &) {

}

QualityInfo ImageQuality::fetchQualityRes(const QualityInputStruct &inputArgs, const Ice::Current &) {

}
