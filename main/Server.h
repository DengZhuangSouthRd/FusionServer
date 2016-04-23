#ifndef _SERVER_H
#define _SERVER_H

#include <iostream>
#include <string>
#include <map>
#include <vector>

#include <Ice/Ice.h>
#include "./src/rpc/wisefuseRpc.h"
#include "./src/utils/log.h"
#include "./src/utils/utils.h"
#include "./src/threadpool/ThreadPool.h"

using namespace std;
using namespace RPCWiseFuse;

class ImageFusion : public WiseFusionInf {
public:
    ImageFusion();
    ~ImageFusion();

public:
    //提交同步任务， 一直等待结果
    //k:id	v:		//任务唯一编号
    //k:panurl	v:	//图像1地址
    //k:msurl	v:	//图像2地址
    //k:outurl	v:	//输出图像地址
    //k:idalg	v:	//算法编号
    //k:band	v:	//波段(1,2,3)
    //k:idinter	v:	//插值算法编号
    virtual ::RPCWiseFuse::FusionInf fuseSyn(const ::RPCWiseFuse::DirArgs& mapArgs, const Ice::Current& );
    //提交异步任务
    virtual int fuseAsyn(const ::RPCWiseFuse::DirArgs& mapArgs, const Ice::Current& );
    //k:id	v:		//任务唯一编号
    virtual string askProcess(const ::RPCWiseFuse::DirArgs& mapArgs, const Ice::Current& );
    //k:id	v:		//任务唯一编号
    virtual ::RPCWiseFuse::FusionInf fetchFuseRes(const ::RPCWiseFuse::DirArgs& mapArgs, const Ice::Current& );

public:
    bool checkFusionArgv(::RPCWiseFuse::DirArgs mapArgs, string& logPath, FusionArgs& args);
    void updateStructInfo(FusionStruct srcInf, FusionInf& destInf);
    void log_InputParameters(DirArgs mapArgs);

private:
    string m_logPath;
    ThreadPool m_threadPool;
};


class Server {
public:
    Server();
    ~Server();

public:
    void initRpc(int argc, char** argv, string conn);
    void restart();
    void close();

private:
    int status;
    Ice::CommunicatorPtr ic;
    int m_argc;
    char** m_argv;
    string connParam;
    Ice::ObjectAdapterPtr adapter;

private:

};

#endif
