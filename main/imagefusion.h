#ifndef IMAGEFUSION_H
#define IMAGEFUSION_H

#include <iostream>
#include <string>
#include <map>
#include <vector>

#include <signal.h>
#include <stdio.h>

#include <Ice/Ice.h>
#include "./src/rpc/wisefuseRpc.h"
#include "./src/utils/log.h"
#include "./src/utils/utils.h"
#include "./src/threadpool/ThreadPool.h"

using namespace std;
using namespace RPCWiseFuse;

typedef struct _TaskStaticResult {
    string task_id;
    FusionArgs input;
    FusionInf output;
}TaskStaticResult;

class ImageFusion : public WiseFusionInf {
public:
    ImageFusion();
    ~ImageFusion();

public:
    //提交同步任务， 一直等待结果
    virtual ::RPCWiseFuse::FusionInf fuseSyn(const ::RPCWiseFuse::DirArgs& mapArgs, const Ice::Current& );
    //提交异步任务
    virtual int fuseAsyn(const ::RPCWiseFuse::DirArgs& mapArgs, const Ice::Current& );
    //k:id	v:		//任务唯一编号
    virtual string askProcess(const ::RPCWiseFuse::DirArgs& mapArgs, const Ice::Current& );
    //k:id	v:		//任务唯一编号
    virtual ::RPCWiseFuse::FusionInf fetchFuseRes(const ::RPCWiseFuse::DirArgs& mapArgs, const Ice::Current& );

public:
    bool checkFusionArgv(::RPCWiseFuse::DirArgs mapArgs, FusionArgs& args);
    void log_InputParameters(DirArgs mapArgs);
    void log_OutputResult(const FusionInf& destInf);

    int getSerializeTaskResults(string serializePath);
    int serializeTaskResults(string serializePath, string serializePathBak);
    void fillFinishTaskMap(const string& task_id, const FusionArgs& inParam, const FusionInf& outParam);
    bool packTaskStaticStatus(TaskStaticResult &res, const string task_id, TaskPackStruct& tmp);

private:
    ThreadPool* p_threadPool;

    string m_serializePath;
    string m_serializePathBak;

    map<string, TaskStaticResult> m_finishMap; // for <task_id, task_all_parameters>
};


#endif // IMAGEFUSION_H
