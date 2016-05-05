#ifndef IMAGEQUALITY_H
#define IMAGEQUALITY_H
#include <iostream>
#include <string>
#include <map>
#include <vector>

#include <signal.h>
#include <stdio.h>

#include <Ice/Ice.h>
#include "./src/rpc/qualityjudgeRpc.h"
#include "./src/utils/log.h"
#include "./src/threadpool/ThreadPool.h"
#include "./src/algorithm/quality/utils/qualityUtils.h"

using namespace std;
using namespace RPCQualityJudge;

class ImageQuality : public QualityInf {
public:
    ImageQuality() ;
    ~ImageQuality() ;

public:
    virtual QualityInfo qualitySyn(const ::RPCQualityJudge::QualityInputStruct& inputArgs, const Ice::Current& );
    virtual int qualityAsyn(const ::RPCQualityJudge::QualityInputStruct& inputArgs, const Ice::Current& );
    virtual QualityInfo fetchQualityRes(const ::RPCQualityJudge::QualityInputStruct& inputArgs, const Ice::Current& );

public:
    bool checkQualityArgv(const ::RPCQualityJudge::QualityInputStruct& inputArgs);
    void log_InputParameters(const ::RPCQualityJudge::QualityInputStruct& inputArgs);
    void log_OutputResult(const ::RPCQualityJudge::QualityInfo& outRes);

private:
    ThreadPool* p_threadPool;
};

#endif // IMAGEQUALITY_H
