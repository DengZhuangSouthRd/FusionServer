#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>

#include <gdal_priv.h>

#include <iostream>
#include <map>
#include "../../../utils/log.h"
#include "../../../rpc/quality/qualityjudgeRpc.h"
#include "../../../imagequality.h"

using namespace std;
using namespace RPCQualityJudge;

//statement for integer data length.
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;

// write status into log file
void WriteMsg(char*, int32_t statusnum, const char* statusmsg);

typedef struct _QualityResMap{
    int status;
    map<string, double> res;
}QualityResMap;

// algorithm class
bool mainClarity(ImageParameter &testparameter, char* logfilepath, double &m_qRes);
bool mainContrastRatio(ImageParameter &testparameter, char* logfilepath, double &m_qRes);
bool mainEntropy(ImageParameter &testparameter, char* logfilepath, double &m_qRes);
bool mainMean(ImageParameter &testparameter, char* logfilepath, double &m_qRes);
bool mainSignaltoNoiseRatio(ImageParameter &testparameter, char* logfilepath, double &m_qRes);
bool mainStriperesidual(ImageParameter &testparameter, char* logfilepath, double &m_qRes);

// interface
void *qualityInterface(void* args);

// set intertimer to serialize the ImageQuality Data !
void utils_serialize_quality(int);
void serializeImageQualityOnTime(int seconds);

#endif // UTILS_H
