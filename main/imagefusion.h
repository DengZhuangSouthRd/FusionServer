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
    bool checkFusionArgv(::RPCWiseFuse::DirArgs mapArgs, FusionArgs& args);
    void log_InputParameters(DirArgs mapArgs);
    void log_OutputResult(const FusionInf& destInf);

private:
    ThreadPool m_threadPool;
};


#endif // IMAGEFUSION_H
