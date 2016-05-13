#include "imageretrieve.h"

extern map<string, string> g_ConfMap;

ImageRetrieve::ImageRetrieve() {
    p_pgdb = NULL;

    string str_pgConf = getPGConfInfo(g_ConfMap);
    Log::Info("PGCONF=" + str_pgConf);
    p_pgdb = new(std::nothrow) PGDB(str_pgConf);
    if(p_pgdb == NULL) {
        Log::Error("Connect PGDB Error !");
        throw "Please Check PGDB Config !";
    }
}

ImageRetrieve::~ImageRetrieve() {
    if(p_pgdb != NULL) {
        delete p_pgdb;
    }
}

WordWiki ImageRetrieve::wordGetKnowledge(const string& word, const Ice::Current &) {
    WordWiki wiki;
    wiki.key = "";
    wiki.abstr = "";
    wiki.descr = "";
    return wiki;
}

WordRes ImageRetrieve::wordSearch(const DictStr2Str& mapArg, const Ice::Current&) {
    ListString str;
    for(int i=1;i<10;++i) {
        str.push_back(to_string(i));
    }
}

ImgRes ImageRetrieve::wordSearchImg(const DictStr2Str& mapArg, const Ice::Current&) {
    ImgRes res;
    for(int i=0;i<10;++i) {
        res.imgRemote.push_back(to_string(i));
        res.imgPic.push_back(to_string(i));
    }
    return res;
}

ImgRes ImageRetrieve::imgSearch(const DictStr2Str& mapArg, const Ice::Current&) {
    ImgRes res;
    for(int i=0;i<10;++i) {
        res.imgRemote.push_back(to_string(i));
        res.imgPic.push_back(to_string(i));
    }
    return res;
}


void ImageRetrieve::log_InputParameters(const DictStr2Str &inputArgs) {
    string str = "";
    for(DictStr2Str::const_iterator it=inputArgs.begin(); it!=inputArgs.end(); it++) {
        str += (it->first + "=" + it->second + "#");
    }
    Log::Info(str);
}

void ImageRetrieve::log_OutputResult(void *outputRes, const string typeKind) {
    string str = "";
    if(typeKind == "WordWiki") {
        WordWiki tmp = *(WordWiki*)outputRes;
        str = "abst=" + tmp.abstr + "#descr=" + tmp.descr + "#key=" + tmp.key;
    }
    if(typeKind == "WordRes") {
        WordRes tmp = *(WordRes*)outputRes;

    }
}
