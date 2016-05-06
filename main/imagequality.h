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

typedef struct _QualityTaskStaticResult{
    string task_id;
    QualityInputStruct input;
    QualityInfo output;
}QualityTaskStaticResult;

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

    void fillFinishTaskMap(const string &task_id, const QualityInputStruct &inParam, const QualityInfo &outParam);
    bool packTaskStaticStatus(QualityTaskStaticResult &res, const string task_id, TaskPackStruct &tmp);

    int getSerializeTaskResults(string serializePath);
    int serializeTaskResults(string serializePath, string serializePathBak);

private:
    ThreadPool* p_threadPool;

    string m_serializePath;
    string m_serializePathBak;

    map<string, QualityTaskStaticResult> m_finishMap; // for <task_id, task_all_parameters>
};

#endif // IMAGEQUALITY_H
