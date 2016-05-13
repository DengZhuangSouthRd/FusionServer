#ifndef IMAGERPCI_H
#define IMAGERPCI_H

#include <iostream>
#include <string>
#include <map>
#include <vector>

#include <signal.h>
#include <stdio.h>

#include <Ice/Ice.h>
#include "./src/rpc/retrieve/ImageRpc.h"
#include "./src/utils/log.h"
#include "./src/threadpool/ThreadPool.h"
#include "./src/algorithm/retrieve/utils/retrieveutils.h"
#include "./src/algorithm/retrieve/utils/pgdb.h"

using namespace std;
using namespace RPCImgRecong;

enum TypeKind {
    TypeWordWiki,
    TypeWordRes,
    TypeImgRes
};

class ImageRetrieve : public ImgRetrieval {
public:
    ImageRetrieve();
    ~ImageRetrieve();

public:
    // 这个方法不用实现，我只需要将接口写在这里就好
    virtual WordWiki wordGetKnowledge(const string& word, const Ice::Current& );

    // this should not working as async method
    // 通过单词搜索得到单词对象列表
    virtual WordRes wordSearch(const ::RPCImgRecong::DictStr2Str& mapArg, const Ice::Current& );

    // this should not working as async method
    // 通过单词搜索得到图像地址列表
    virtual ImgRes wordSearchImg(const ::RPCImgRecong::DictStr2Str& mapArg, const Ice::Current& );

    // this should working as async method, through the thread working
    // 通过图像搜索得到图像地址列表,同步
    virtual ImgRes imgSearchSync(const ::RPCImgRecong::DictStr2Str& mapArg, const Ice::Current& );

    // 异步提交任务
    virtual int imgSearchAsync(const ::RPCImgRecong::DictStr2Str& mapArg, const Ice::Current& );
    virtual ImgRes fetchImgSearchResult(const ::RPCImgRecong::DictStr2Str& mapArg, const Ice::Current& );

public:
    void log_InputParameters(const ::RPCImgRecong::DictStr2Str& inputArgs);
    void log_OutputResult(void* outputRes, TypeKind paramType);

private:
    PGDB* p_pgdb;
    string m_logPath;
};
#endif // IMAGERPCI_H
