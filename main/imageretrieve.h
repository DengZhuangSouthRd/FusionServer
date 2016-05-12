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

using namespace std;
using namespace RPCImgRecong;

class ImageRetrieve : public ImgRetrieval {
public:
    ImageRetrieve();
    ~ImageRetrieve();

public:
    // this should not working as async method
    virtual WordWiki wordGetKnowledge(const string& word, const Ice::Current& );
    // this should not working as async method
    virtual ListString wordSearch(const ::RPCImgRecong::DictStr2Str& mapArg, const Ice::Current& );
    // this should not working as async method
    virtual ImgRes wordSearchImg(const ::RPCImgRecong::DictStr2Str& mapArg, const Ice::Current& );
    // this should working as async method, through the thread working
    virtual ImgRes imgSearch(const ::RPCImgRecong::DictStr2Str& mapArg, const Ice::Current& );

private:
    PGDB* p_pgdb;
    string m_logPath;
};
#endif // IMAGERPCI_H
