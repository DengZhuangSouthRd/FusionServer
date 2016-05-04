#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <gdal_priv.h>

#include <iostream>
#include "../../../utils/log.h"

using namespace std;

//statement for integer data length.
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;
typedef long int int64_t;
//typedef unsigned long long uint64_t;

//Image Parameters for Radiation quality Evaluation
typedef struct
{
    string filePath;
    int32_t rowNum;
    int32_t columnNum;
    int32_t bandNum;
    int32_t bitsPerPixel;//8、16、24
}ImageParameter;

//write status into log file
void WriteMsg(char*, int32_t statusnum, char* statusmsg);
bool read_ConfigureFile_Parameters(const char* parafilepath, ImageParameter& testparameter);

// algorithm class
bool mainClarity(char* parafilepath, char* logfilepath);
bool mainContrastRatio(char* parafilepath, char* logfilepath);
bool mainEntropy(char* parafilepath, char* logfilepath);
bool mainMean(char* parafilepath, char* logfilepath);
bool mainSignaltoNoiseRatio(char* parafilepath, char* logfilepath);
bool mainStriperesidual(char* parafilepath, char* logfilepath);

//interface
void *qualityInterface();

#endif // UTILS_H

