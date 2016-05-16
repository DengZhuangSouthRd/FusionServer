#include "qualityutils.h"

extern ImageQuality* g_ImgQuality;

//write status into log file
void WriteMsg(char* , int32_t statusnum, const char* statusmsg) {
    Log::Info("%d\t%s", statusnum, statusmsg);
}

bool createQualityRes(QualityRes &quaRes, int length) {
    quaRes.status = -1;
    quaRes.data = NULL;
    quaRes.data = (double*)malloc(sizeof(double)*length);
    quaRes.length = length;
    if(quaRes.data == NULL) {
        return false;
    }
    return true;
}

void revokeQualityRes(QualityRes **p_quaRes) {
    if((*p_quaRes) == NULL)
        return ;
    if((*p_quaRes)->data != NULL)
        free((*p_quaRes)->data);
    (*p_quaRes)->data = NULL;
    (*p_quaRes)->length = 0;
    (*p_quaRes)->status = -1;
    free(*p_quaRes);
    (*p_quaRes) = NULL;
}

void deepCopyQualityRes2Info(const QualityRes &src, QualityInfo &dest) {
    dest.status = src.status;
    string id = "first";
    DataArray tmp;
    for(int i=0; i<src.length; i++) {
        tmp.push_back(src.data[i]);
    }
    dest.imgsquality[id] = tmp;
}

void utils_serialize_quality(int ) {
    if(NULL != g_ImgQuality) {
        g_ImgQuality->serializeTaskResults();
    }
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
    if(args == NULL) return NULL;
    QualityInputStruct * tmp = (QualityInputStruct*)args;

    int algorithmClass = tmp->algorithmkind;
    char* logfilepath = NULL;
    ImageParameter testparameter;
    if(tmp->inputMap.size() == 1) {
        for(QualityMapArgs::iterator it=tmp->inputMap.begin(); it!=tmp->inputMap.end(); it++) {
            testparameter = it->second;
        }
    }

    QualityRes* p_quaRes = new(std::nothrow) QualityRes;
    bool flag = false;
    flag = createQualityRes(*p_quaRes, testparameter.bandNum);
    if(flag == false) {
        Log::Info("Create Quality Struct Failed !");
        return NULL;
    }

    switch (algorithmClass) {
    case 1:
        flag = mainClarity(testparameter, logfilepath, *p_quaRes);
        break;
    case 2:
        flag = mainContrastRatio(testparameter, logfilepath, *p_quaRes);
        break;
    case 3:
        flag = mainEntropy(testparameter, logfilepath, *p_quaRes);
        break;
    case 4:
        flag = mainMean(testparameter, logfilepath, *p_quaRes);
        break;
    case 5:
        flag = mainSignaltoNoiseRatio(testparameter, logfilepath, *p_quaRes);
        break;
    case 6:
        flag = mainStriperesidual(testparameter, logfilepath, *p_quaRes);
        break;
    default:
        return NULL;
    }
    return (void*)p_quaRes;
}
