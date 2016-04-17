#ifndef _SERVER_H
#define _SERVER_H

#include <iostream>
#include <string>
#include <map>
#include <vector>

#include <Ice/Ice.h>
#include "./src/rpc/wisefuseRpc.h"
#include "./src/utils/log.h"
#include "./src/utils/utils.hpp"
#include "./src/threadpool/ThreadPool.h"

using namespace std;
using namespace RPCWiseFuse;

/*
struct FusionInf{
    //数据库写入信息
    double	cnttimeuse;				//所用时间
    string	producetime;			//产品生成时间
    double	ulcoorvalidLongitude;	//有效融合区域左上角经度坐标
    double	ulcoorvalidLatitude ;	//有效融合区域左上角纬度坐标
    double	brcoorvalidLongitude;	//有效融合区域右下角经度坐标
    double	brcoorvalidLatitude ;	//有效融合区域右下角纬度坐标
    double	ulcoorwholeLongitude;	//全局左上角经度坐标
    double	ulcoorwholeLatitude ;	//全局左上角纬度坐标 
    double	brcoorwholeLongitude;	//全局右下角经度坐标
    double	brcoorwholeLatitude ;	//全局右下角纬度坐标
    float	resolution;				//融合后产品分辨率
    string	productFormat;			//产品格式
    string	projectiontype;			//投影类型
    string	datumname;				//投影椭球体
    string	projectioncode;			//投影编码
    string	projectionunits;		//投影单位
    float	projcentralmeridian;	//投影带中央经线 
    int		status;					//任务执行状态	
};
 */

typedef struct _FusionArgs {
    string panurl;
    string msurl;
    string outurl;
    string logurl;
    int idalg;
    vector<int> band;
    int idinter;
}FusionArgs;

enum TaskStatus {
    FINISHED,
    SUCCESS,
    TASKCRASH,
    ARGERROR
};

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
    virtual FusionInf fuseSyn(const ::RPCWiseFuse::DirArgs& mapArgs, const Ice::Current& );
    //提交异步任务
    virtual int fuseAsyn(const ::RPCWiseFuse::DirArgs& mapArgs, const Ice::Current& );
    //k:id	v:		//任务唯一编号
    virtual string askProcess(const ::RPCWiseFuse::DirArgs& mapArgs, const Ice::Current& );
    //k:id	v:		//任务唯一编号
    virtual FusionInf fetchFuseRes(const ::RPCWiseFuse::DirArgs& mapArgs, const Ice::Current& );

public:
    bool checkFusionArgv(::RPCWiseFuse::DirArgs mapArgs, string& logPath, FusionArgs& args);
    void setLogPath(string logPath);

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
