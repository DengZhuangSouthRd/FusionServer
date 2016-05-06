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
    str += ("status="+to_string(outRes.status)+"#");
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
    QualityInfo quaRes;
    quaRes.status = -1;
    bool flag = checkQualityArgv(inputArgs);
    if(flag == false)
        return quaRes;
    QualityRes* tmp = NULL;
    tmp = (QualityRes*)qualityInterface((void*)(&inputArgs));
    if(tmp == NULL) {
        return quaRes;
    }
    string task_id = inputArgs.id;
    quaRes.status = 1;
    quaRes.imgsquality[task_id] = vector<double>();
    for(int i=0;i<tmp->length;i++) {
        quaRes.imgsquality[task_id].push_back(tmp->data[i]);
    }
    revokeQualityRes(&tmp);
    return quaRes;
}

int ImageQuality::qualityAsyn(const QualityInputStruct &inputArgs, const Ice::Current &) {

}

QualityInfo ImageQuality::fetchQualityRes(const QualityInputStruct &inputArgs, const Ice::Current &) {

}

void ImageQuality::fillFinishTaskMap(const string &task_id, const QualityInputStruct &inParam, const QualityInfo &outParam) {
    if(m_finishMap.count(task_id) == 0) {
        QualityTaskStaticResult tmp;
        tmp.task_id.assign(task_id);
        tmp.input = inParam;
        tmp.output = outParam;
        m_finishMap[task_id] = tmp;
        Log::Info("Finish Task size is %d !", m_finishMap.size());
    }
}

bool ImageQuality::packTaskStaticStatus(QualityTaskStaticResult &res, const string task_id, TaskPackStruct &tmp) {
    QualityInfo* out_res = (QualityInfo*)tmp.output;
    if(out_res->status <= 0) {
        return false;
    }

    res.task_id.assign(task_id);

    res.output.status = out_res->status;
    for(DatasMap::iterator it=out_res->imgsquality.begin(); it!=out_res->imgsquality.end(); it++) {
        DataArray tmp;
        for(DataArray::iterator vit=it->second.begin(); vit!=it->second.end(); vit++) {
            tmp.push_back(*vit);
        }
        res.output.imgsquality[it->first] = tmp;
    }

    QualityInputStruct* param = (QualityInputStruct*) tmp.input;
    res.input.id.assign(param->id);
    res.input.algorithmkind = param->algorithmkind;
    for(QualityMapArgs::iterator it=param->inputMap.begin(); it!=param->inputMap.end(); it++) {
        ImageParameter tmp;
        tmp.filePath = it->second.filePath;
        tmp.colNum = it->second.colNum;
        tmp.rowNum = it->second.rowNum;
        tmp.bandNum = it->second.bandNum;
        tmp.bitsPerPixel = it->second.bitsPerPixel;
        res.input.inputMap[it->first] = tmp;
    }

    return true;
}
