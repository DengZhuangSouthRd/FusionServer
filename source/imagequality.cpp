#include "imagequality.h"

extern map<string, string> g_ConfMap;
ImageQuality* g_ImgQuality = NULL;

ImageQuality::ImageQuality() {
    m_serializePath = g_ConfMap["QUALTYSerializePath"];
    m_serializePathBak = g_ConfMap["QUALTYSerializePathBak"];



    getSerializeTaskResults(m_serializePath);
    p_threadPool = NULL;
    p_threadPool = ThreadPool::getSingleInstance();
    g_ImgQuality = this;
    serializeImageQualityOnTime(atoi(g_ConfMap["SERIALIZETIME"].c_str()));
}

ImageQuality::~ImageQuality() {
    g_ImgQuality = NULL;
    p_threadPool->revokeSingleInstance();
    p_threadPool = NULL;
}

bool ImageQuality::checkQualityArgv(const QualityInputStruct &inputArgs) {
    // main to change the filePath is Right !
    for(QualityMapArgs::const_iterator it=inputArgs.inputMap.begin(); it!=inputArgs.inputMap.end(); it++) {
        if(!(it->second.bitsPerPixel == 8 || it->second.bitsPerPixel == 16 || it->second.bitsPerPixel == 24)) {
            Log::Error("BitsPerPixel Value not in 8, 16, 24 ! Please Check !");
            return false;
        }
        if(it->second.bandNum <= 0) {
            Log::Error("BandNum Value <= 0 ! Please Check !");
            return false;
        }
        if(it->second.colNum <= 0 || it->second.rowNum <= 0) {
            Log::Error("ColNum <=0 Or RowNum <= 0 !");
            return false;
        }
        if(access(it->second.filePath.c_str(), 0) != 0) {
            Log::Error("FilePath Not Exists ! Please Check !");
            return false;
        }
    }
    return true;
}

void ImageQuality::log_InputParameters(const QualityInputStruct &inputArgs) {
    string str = "";
    str += ("task_id="+inputArgs.id+"#");
    str += ("algorithmkind="+inputArgs.algorithmkind+"#");
    for(QualityMapArgs::const_iterator it=inputArgs.inputMap.begin(); it!=inputArgs.inputMap.end(); it++) {
        str += (it->first+"$"+"filePath="+it->second.filePath+"#rowNum="+to_string(it->second.rowNum)+"#colNum="+to_string(it->second.colNum));
        str += ("#bandNum="+to_string(it->second.bandNum));
        str += ("#bitsPerPixel="+to_string(it->second.bitsPerPixel));
    }
    Log::Info(str);
}

void ImageQuality::log_OutputResult(const QualityInfo &outRes) {
    string str = "";
    str += ("status="+to_string(outRes.status)+"#");
    for(DatasMap::const_iterator it=outRes.imgsquality.begin(); it!=outRes.imgsquality.end(); it++) {
        str += ("key="+it->first+"#");
        str += "bandValue=";
        for(DataArray::const_iterator vit=it->second.begin(); vit!= it->second.end(); vit++) {
            str += (to_string(*vit)+",");
        }
    }
    Log::Info(str);
}

QualityInfo ImageQuality::qualitySyn(const QualityInputStruct &inputArgs, const Ice::Current &) {
    string task_id = inputArgs.id;
    if(m_finishMap.count(task_id) != 0) {
        return m_finishMap[task_id].output;
    }

    QualityInfo quaRes;
    quaRes.status = -1;
    bool flag = checkQualityArgv(inputArgs);
    if(flag == false)
        return quaRes;
    QualityRes* tmp = NULL;
    tmp = (QualityRes*)qualityInterface((void*)(&inputArgs));
    if(tmp == NULL) {
        return quaRes;
    }

    quaRes.status = 1;
    quaRes.imgsquality[task_id] = vector<double>();
    for(int i=0;i<tmp->length;i++) {
        quaRes.imgsquality[task_id].push_back(tmp->data[i]);
    }
    revokeQualityRes(&tmp);
    fillFinishTaskMap(task_id, inputArgs, quaRes);
    return quaRes;
}

int ImageQuality::qualityAsyn(const QualityInputStruct &inputArgs, const Ice::Current &) {
    string task_id = inputArgs.id;

    if(m_finishMap.count(task_id) != 0) {
        return 1;
    }

    QualityInputStruct* args = new(std::nothrow) QualityInputStruct;
    if(args == NULL) {
        Log::Error("qualityAsyn ## new QualityInputStruct Failed !");
        return -1;
    }

    log_InputParameters(inputArgs);
    bool flag = checkQualityArgv(inputArgs);
    if(flag == false) {
        delete args;
        Log::Error("qualityAsyn ## Image ImageQuality Parameters Error !");
        return -1; // push task error
    }

    Task* task = new(std::nothrow) Task(&qualityInterface, (void*)args);
    if(task == NULL) {
        delete args;
        Log::Error("qualityAsyn ## new Task Failed !");
        return -1;
    }

    task->setTaskID(task_id);
    if(p_threadPool->add_task(task, task_id) != 0) {
        Log::Error("qualityAsyn ## thread Pool add Task Failed !");
        delete args;
        delete task;
        return -1; // Means For Add Task Failed !
    }
    return 1; // Means For Add Task Success !
}

QualityInfo ImageQuality::fetchQualityRes(const QualityInputStruct &inputArgs, const Ice::Current &) {
    log_InputParameters(inputArgs);
    QualityInfo obj;
    string task_id = inputArgs.id;

    if(m_finishMap.count(task_id) != 0) {
        return m_finishMap[task_id].output;
    }

    TaskPackStruct tmp;
    bool flag = p_threadPool->fetchResultByTaskID(task_id, tmp);
    if(flag == false) {
        obj.status = -1;
        Log::Error("fetchQualityRes ## fetch task id %s result Failed !", task_id.c_str());
    } else {
        deepCopyQualityRes2Info(*(QualityRes*)(tmp.output), obj);
        log_OutputResult(obj);
        QualityTaskStaticResult res;
        flag = packTaskStaticStatus(res, task_id, tmp);
        delete (QualityInputStruct*)tmp.input;
        revokeQualityRes((QualityRes**)&(tmp.output));
        if(flag == true) {
            m_finishMap[task_id] = res;
        }
    }
    return obj;
}

void ImageQuality::fillFinishTaskMap(const string &task_id, const QualityInputStruct &inParam, const QualityInfo &outParam) {
    if(m_finishMap.count(task_id) == 0) {
        QualityTaskStaticResult tmp;
        tmp.task_id.assign(task_id);
        tmp.input = inParam;
        tmp.output = outParam;
        m_finishMap[task_id] = tmp;
        Log::Info("Finish Task size is %d !", m_finishMap.size());
    }
}

bool ImageQuality::packTaskStaticStatus(QualityTaskStaticResult &res, const string task_id, TaskPackStruct &tmp) {
    QualityInfo* out_res = (QualityInfo*)tmp.output;
    if(out_res->status <= 0) {
        return false;
    }

    res.task_id.assign(task_id);

    res.output.status = out_res->status;
    for(DatasMap::iterator it=out_res->imgsquality.begin(); it!=out_res->imgsquality.end(); it++) {
        DataArray tmp;
        for(DataArray::iterator vit=it->second.begin(); vit!=it->second.end(); vit++) {
            tmp.push_back(*vit);
        }
        res.output.imgsquality[it->first] = tmp;
    }

    QualityInputStruct* param = (QualityInputStruct*) tmp.input;
    res.input.id.assign(param->id);
    res.input.algorithmkind = param->algorithmkind;
    for(QualityMapArgs::iterator it=param->inputMap.begin(); it!=param->inputMap.end(); it++) {
        ImageParameter tmp;
        tmp.filePath = it->second.filePath;
        tmp.colNum = it->second.colNum;
        tmp.rowNum = it->second.rowNum;
        tmp.bandNum = it->second.bandNum;
        tmp.bitsPerPixel = it->second.bitsPerPixel;
        res.input.inputMap[it->first] = tmp;
    }

    return true;
}


// get the over task from the Json File
int ImageQuality::getSerializeTaskResults(string serializePath) {
    Json::Reader reader;
    Json::Value root;
    Json::Value::Members members;
    std::ifstream in;
    in.open(serializePath.c_str(), std::ios_base::binary);
    if(in.is_open() == false) {
        throw runtime_error("Open Serialize File Error !");
        cerr << "Open Seriazlize file Error !" << endl;
    }
    bool flag = reader.parse(in,root, false);
    if(flag == false) {
        throw runtime_error("Parse Serialize Json File failed !");
        cerr << "Parse Serialize Json File failed !" << endl;
    }
    members = root.getMemberNames();
    for(Json::Value::Members::iterator it=members.begin(); it!=members.end(); ++it) {
        std::string key = *it;
        Json::Value node = root[key];
        Json::Value inNode = node[0];
        Json::Value outNode = node[1];

        QualityTaskStaticResult tmp;
        tmp.task_id.assign(key);
        tmp.input.id.assign(inNode.get("id", "NULL").asString());
        tmp.input.algorithmkind = inNode.get("algorithmkind", 1).asInt();
        QualityMapArgs inMap;
        Json::Value::Members inNodeMembers = inNode.getMemberNames();
        for(Json::Value::Members::iterator inIt=inNodeMembers.begin(); inIt!=inNodeMembers.end(); inIt++) {
            std::string inKey = *inIt;
            if(inKey == "id" || inKey == "algorithmkind")
                continue;
            ImageParameter a;
            Json::Value imageArr = inNode[inKey][0];
            a.filePath.assign(imageArr.get("filePath", "NULL").asString());
            a.rowNum = imageArr.get("rowNum", 0).asInt();
            a.colNum = imageArr.get("colNum", 0).asInt();
            a.bandNum = imageArr.get("bandNum", 0).asInt();
            a.bitsPerPixel = imageArr.get("bitsPerPixel", 0).asInt();
            inMap[inKey] = a;
        }
        tmp.input.inputMap = inMap;

        Json::Value::Members outNodeMembers = outNode.getMemberNames();
        tmp.output.status = outNode.get("status", 0).asInt();
        for(Json::Value::Members::iterator outIt=outNodeMembers.begin(); outIt!=outNodeMembers.end(); outIt++) {
            std::string outKey = *outIt;
            if(outKey == "status")
                continue;
            DataArray tmpArr;
            Json::Value bandValueArr = outNode[outKey];
            for(unsigned int i=0;i<bandValueArr.size();i++) {
                tmpArr.push_back(bandValueArr[i].asDouble());
            }
            tmp.output.imgsquality[outKey] = tmpArr;
        }

        m_finishMap[key] = tmp;
    }
    in.close();
    return members.size();
}

//fetch all task id and task result to serialize the completed task !
void ImageQuality::serializeTaskResults() {

    Json::FastWriter writer;
    Json::Value root;

    for(map<string, QualityTaskStaticResult>::iterator it=m_finishMap.begin(); it!=m_finishMap.end(); ++it) {
        string key = it->first;
        QualityTaskStaticResult res = it->second;

        Json::Value input;
        input["id"] = res.input.id;
        input["algorithmkind"] = res.input.algorithmkind;
        for(QualityMapArgs::iterator it=res.input.inputMap.begin(); it!=res.input.inputMap.end(); it++) {
            Json::Value tmp;
            tmp["filePath"] = it->second.filePath;
            tmp["colNum"] = it->second.colNum;
            tmp["rowNum"] = it->second.rowNum;
            tmp["bandNum"] = it->second.bandNum;
            tmp["bitsPerPixel"] = it->second.bitsPerPixel;
            input[it->first] = tmp;
        }

        Json::Value outres;
        outres["status"] = res.output.status;
        for(DatasMap::iterator it=res.output.imgsquality.begin(); it!=res.output.imgsquality.end(); it++) {
            Json::Value tmp;
            for(DataArray::iterator vit=it->second.begin(); vit!=it->second.end(); vit++) {
                tmp.append(*vit);
            }
            outres[it->first] = tmp;
        }

        root[key].append(input);
        root[key].append(outres);
    }
    std::string strRoot = writer.write(root);

    std::ofstream out;
    out.open(m_serializePathBak.c_str(), std::ios_base::binary);
    if(out.is_open() == false) {
        throw runtime_error("Open Serialize Bak File Error !");
        cerr << "Open Seriazlize Bak file Error !" << endl;
    }
    out << strRoot;
    out.close();

    out.open(m_serializePath.c_str(), std::ios_base::binary);
    if(out.is_open() == false) {
        throw runtime_error("Open Serialize File Error !");
        cerr << "Open Seriazlize file Error !" << endl;
    }
    out << strRoot;
    out.close();
}
