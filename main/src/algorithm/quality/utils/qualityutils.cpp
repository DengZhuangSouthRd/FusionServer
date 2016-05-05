#include "qualityUtils.h"

//write status into log file
void WriteMsg(char* ,int32_t statusnum, char* statusmsg) {
    Log::Info("%d\t%s", statusnum, statusmsg);
}

void* qualityInterface(void *args) {
    char* logfilepath = NULL;
    int algorithmClass = 1;

    ImageParameter testparameter;
    QualityRes quaRes;

    bool flag = false;

    switch (algorithmClass) {
    case 1:
        flag = mainClarity(testparameter, logfilepath, quaRes);
        break;
    case 2:
        flag = mainContrastRatio(testparameter, logfilepath, quaRes);
        break;
    case 3:
        flag = mainEntropy(testparameter, logfilepath, quaRes);
        break;
    case 4:
        flag = mainMean(testparameter, logfilepath, quaRes);
        break;
    case 5:
        flag = mainSignaltoNoiseRatio(testparameter, logfilepath, quaRes);
        break;
    case 6:
        flag = mainStriperesidual(testparameter, logfilepath, quaRes);
        break;
    default:
        return NULL;
    }
    return NULL;
}
