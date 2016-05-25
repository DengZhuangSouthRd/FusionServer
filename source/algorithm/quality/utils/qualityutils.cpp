#include "qualityutils.h"

//write status into log file
void WriteMsg(char* , int32_t statusnum, const char* statusmsg) {
    Log::Info("%d\t%s", statusnum, statusmsg);
}

void utils_serialize_quality(int ) {
;
}

void serializeImageQualityOnTime(int seconds) {
    struct itimerval tick;
    memset(&tick, 0, sizeof(tick));
    tick.it_interval.tv_sec = seconds;
    tick.it_interval.tv_usec = 0;
    tick.it_value.tv_sec = seconds;
    tick.it_value.tv_usec = 0;
    signal(SIGALRM, utils_serialize_quality);
    if(setitimer(ITIMER_REAL, &tick, NULL) < 0) {
        Log::Error("Set Timer to serialize ImageQuality Dict Failed !");
        throw runtime_error("Set Timer to Serialize ImageQuality Dict Failed !");
    }
}

void* qualityInterface(void *args) {
    map<string,int> evaluatealg;
    evaluatealg["Clarity_1_0"]=1;
    evaluatealg["ContrastRatio_1_0"]=2;
    evaluatealg["Entropy_1_0"]=3;
    evaluatealg["Mean_1_0"]=4;
    evaluatealg["SignaltoNoiseRatio_1_0"]=5;
    evaluatealg["Striperesidual_1_0"]=6;

    evaluatealg["CrossEntropy_1_0"]=7;
    evaluatealg["MutualInformation_1_0"]=8;
    evaluatealg["SpectralAngleMatrix_1_0"]=9;
    evaluatealg["StructureSimilarity_1_0"]=10;

    evaluatealg["DynamicRange_1_0"] = 11;
    evaluatealg["Variance_1_0"] = 12;

    if(args == NULL) return NULL;
    QualityInputStruct * tmp = (QualityInputStruct*)args;
    int inputParamSize = 1;
    inputParamSize = tmp->inputMap.size();

    vector<string> inputPathVec;
    vector<int> bandlist;
    inputPathVec.resize(3);
    inputPathVec[0] = tmp->inputMap["f1"].filePath;
    inputPathVec[1] = tmp->inputMap["f2"].filePath;
    inputPathVec[2] = tmp->inputMap["f3"].filePath;

    if(tmp->inputMap.size() == 1) {
        for(QualityMapArgs::iterator it=tmp->inputMap.begin(); it!=tmp->inputMap.end(); it++) {
            inputPathVec.push_back(it->second.filePath);
        }
    } else if(tmp->inputMap.size() == 2) {
        for(QualityMapArgs::iterator it=tmp->inputMap.begin(); it!=tmp->inputMap.end(); it++) {
            inputPathVec.push_back(it->second.filePath);
        }
    }
    QualityResMap* p_resMap = new(std::nothrow) QualityResMap;
    if(p_resMap == NULL) {
        return NULL;
    }
    vector<double> qualityRes;
    bool flag = false;
    char* logfilepath = NULL;
    if(inputParamSize == 1) {
        flag = mainClarity(inputPathVec[2], logfilepath, qualityRes);
        p_resMap->res["Clarity_1_0"] = qualityRes;
        flag = mainContrastRatio(inputPathVec[2], logfilepath, qualityRes);
        p_resMap->res["ContrastRatio_1_0"] = qualityRes;
        flag = mainEntropy(inputPathVec[2], logfilepath, qualityRes);
        p_resMap->res["Entropy_1_0"] = qualityRes;
        flag = mainMean(inputPathVec[2], logfilepath, qualityRes);
        p_resMap->res["Mean_1_0"] = qualityRes;
        flag = mainSignaltoNoiseRatio(inputPathVec[2], logfilepath, qualityRes);
        p_resMap->res["SignaltoNoiseRatio_1_0"] = qualityRes;
        flag = mainStriperesidual(inputPathVec[2], logfilepath, qualityRes);
        p_resMap->res["Striperesidual_1_0"] = qualityRes;

        flag = mainDynamicRange(inputPathVec[2], logfilepath, qualityRes);
        p_resMap->res["DynamicRange_1_0"] = qualityRes;
        flag = mainVariance(inputPathVec[2], logfilepath, qualityRes);
        p_resMap->res["Variance_1_0"] = qualityRes;
    } else if(inputParamSize == 2) {
        flag = mainCrossEntropy(inputPathVec[0], inputPathVec[2], logfilepath, qualityRes);
        p_resMap->res["CrossEntropy_1_0"] = qualityRes;
        flag = mainMutualInformation(inputPathVec[0], inputPathVec[2], logfilepath, qualityRes);
        p_resMap->res["MutualInformation_1_0"] = qualityRes;
        flag = mainSpectralAngleMatrix(inputPathVec[1], inputPathVec[2], logfilepath, bandlist, qualityRes);
        p_resMap->res["SpectralAngleMatrix_1_0"] = qualityRes;
        flag = mainStructureSimilarity(inputPathVec[0], inputPathVec[2], logfilepath, qualityRes);
        p_resMap->res["StructureSimilarity_1_0"] = qualityRes;
    }
    return p_resMap;
}
