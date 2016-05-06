#include "imagefusion.h"

extern map<string, string> g_ConfMap;
ImageFusion* g_ImgFusion = NULL;

ImageFusion::ImageFusion() {

    m_serializePath = g_ConfMap["FUSIONSerializePath"];
    m_serializePathBak = g_ConfMap["FUSIONSerializePathBak"];

    if(isExistsFile(m_serializePath) == false) {
        cerr << "Serialized File Does Not Exists !" << endl;
        Log::Error("Serialized File Does Not Exists !");
        throw runtime_error("Serialized File Does Not Exists !");
    }
    if(isExistsFile(m_serializePathBak) == false) {
        cerr << "Serialized Bak File Does Not Exists !" << endl;
        Log::Error("Serialized Bak File Does Not Exists !");
        throw runtime_error("Serialized Bak File Does Not Exists !");
    }

    getSerializeTaskResults(m_serializePath);

    p_threadPool = NULL;
    p_threadPool = ThreadPool::getSingleInstance();

    g_ImgFusion = this;
    serializeImageFusionOnTime(atoi(g_ConfMap["SERIALIZETIME"].c_str()));
}

ImageFusion::~ImageFusion() {
    g_ImgFusion = NULL;
    p_threadPool->revokeSingleInstance();
    p_threadPool = NULL;
}

void ImageFusion::log_InputParameters(DirArgs mapArgs) {
    string str = "";
    for(DirArgs::iterator it=mapArgs.begin(); it!=mapArgs.end(); ++it) {
        str += (it->first + "=" + it->second + "#");
    }
    Log::Info(str);
}

void ImageFusion::log_OutputResult(const FusionInf &destInf) {
    string str = "brcoorvalidLatitude = " + to_string(destInf.brcoorvalidLatitude)
            + " brcoorvalidLongitude = " + to_string(destInf.brcoorvalidLongitude)
            + " brcoorwholeLatitude = " + to_string(destInf.brcoorwholeLatitude)
            + " brcoorwholeLongitude = " + to_string(destInf.brcoorwholeLongitude)
            + " ulcoorvalidLatitude = " + to_string(destInf.ulcoorvalidLatitude)
            + " ulcoorvalidLongitude = " + to_string(destInf.ulcoorvalidLongitude)
            + " ulcoorwholeLatitude = " + to_string(destInf.ulcoorwholeLatitude)
            + " ulcoorwholeLongitude = " + to_string(destInf.ulcoorwholeLongitude)
            + " cnttimeuse = " + to_string(destInf.cnttimeuse)
            + " datumname = " + destInf.datumname
            + " producetime = " + destInf.producetime
            + " productFormat = " + destInf.productFormat
            + " projcentralmeridian = " + to_string(destInf.projcentralmeridian)
            + " projectioncode = " + destInf.projectioncode
            + " projectiontype = " + destInf.projectiontype
            + " projectionunits = " + destInf.projectionunits
            + " resolution = " + to_string(destInf.resolution)
            + " status = " + to_string(destInf.status);
    Log::Info(str);
}

// get the over task from the Json File
int ImageFusion::getSerializeTaskResults(string serializePath) {
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

        FusionTaskStaticResult tmp;
        tmp.task_id.assign(key);
        tmp.input.panurl.assign(inNode["panurl"].asString());
        tmp.input.outurl.assign(inNode["outurl"].asString());
        tmp.input.msurl.assign(inNode["msurl"].asString());
        tmp.input.logurl.assign(inNode["logurl"].asString());
        tmp.input.idinter = inNode["idinter"].asInt();
        tmp.input.idalg = inNode["idalg"].asInt();
        tmp.input.band.push_back(inNode["band1"].asInt());
        tmp.input.band.push_back(inNode["band2"].asInt());
        tmp.input.band.push_back(inNode["band3"].asInt());

        tmp.output.brcoorvalidLatitude = outNode["brcoorvalidLatitude"].asDouble();
        tmp.output.brcoorvalidLongitude = outNode["brcoorvalidLongitude"].asDouble();
        tmp.output.brcoorwholeLatitude = outNode["brcoorwholeLatitude"].asDouble();
        tmp.output.brcoorwholeLongitude = outNode["brcoorwholeLongitude"].asDouble();
        tmp.output.cnttimeuse = outNode["cnttimeuse"].asDouble();
        tmp.output.datumname.assign(outNode["datumname"].asString());
        tmp.output.producetime.assign(outNode["producetime"].asString());
        tmp.output.productFormat.assign(outNode["productFormat"].asString());
        tmp.output.projcentralmeridian = outNode["projcentralmeridian"].asFloat();
        tmp.output.projectioncode.assign(outNode["projectioncode"].asString());
        tmp.output.projectiontype.assign(outNode["projectiontype"].asString());
        tmp.output.projectionunits.assign(outNode["projectionunits"].asString());
        tmp.output.resolution = outNode["resolution"].asFloat();
        tmp.output.status = outNode["status"].asInt();
        tmp.output.ulcoorvalidLatitude = outNode["ulcoorvalidLatitude"].asDouble();
        tmp.output.ulcoorvalidLongitude = outNode["ulcoorvalidLongitude"].asDouble();
        tmp.output.ulcoorwholeLatitude = outNode["ulcoorwholeLatitude"].asDouble();
        tmp.output.ulcoorwholeLongitude = outNode["ulcoorwholeLongitude"].asDouble();
        m_finishMap[key] = tmp;
    }
    in.close();
    return members.size();
}

//fetch all task id and task result to serialize the completed task !
void ImageFusion::serializeTaskResults() {

    Json::FastWriter writer;
    Json::Value root;

    for(map<string, FusionTaskStaticResult>::iterator it=m_finishMap.begin(); it!=m_finishMap.end(); ++it) {
        string key = it->first;
        FusionTaskStaticResult res = it->second;
        Json::Value input;
        input["panurl"] = res.input.panurl;
        input["outurl"] = res.input.outurl;
        input["msurl"] = res.input.msurl;
        input["logurl"] = res.input.logurl;
        input["idinter"] = res.input.idinter;
        input["idalg"] = res.input.idalg;
        input["band1"] = res.input.band[0];
        input["band2"] = res.input.band[1];
        input["band3"] = res.input.band[2];

        Json::Value outres;
        outres["brcoorvalidLatitude"] = res.output.brcoorvalidLatitude;
        outres["brcoorvalidLongitude"] = res.output.brcoorvalidLongitude;
        outres["brcoorwholeLatitude"] = res.output.brcoorwholeLatitude;
        outres["brcoorwholeLongitude"] = res.output.brcoorwholeLongitude;
        outres["cnttimeuse"] = res.output.cnttimeuse;
        outres["datumname"] = res.output.datumname;
        outres["producetime"] = res.output.producetime;
        outres["productFormat"] = res.output.productFormat;
        outres["projcentralmeridian"] = res.output.projcentralmeridian;
        outres["projectioncode"] = res.output.projectioncode;
        outres["projectiontype"] = res.output.projectiontype;
        outres["projectionunits"] = res.output.projectionunits;
        outres["resolution"] = res.output.resolution;
        outres["status"] = res.output.status;
        outres["ulcoorvalidLatitude"] = res.output.ulcoorvalidLatitude;
        outres["ulcoorvalidLongitude"] = res.output.ulcoorvalidLongitude;
        outres["ulcoorwholeLatitude"] = res.output.ulcoorwholeLatitude;
        outres["ulcoorwholeLongitude"] = res.output.ulcoorwholeLongitude;

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

void ImageFusion::fillFinishTaskMap(const string &task_id, const FusionArgs &inParam, const FusionInf &outParam) {
    if(m_finishMap.count(task_id) == 0) {
        FusionTaskStaticResult tmp;
        tmp.task_id.assign(task_id);
        deepCopyTaskInputParameter(inParam, tmp.input);
        deepCopyTaskResult(outParam, tmp.output);
        m_finishMap[task_id] = tmp;
        Log::Info("Finish Task size is %d !", m_finishMap.size());
    }
}

bool ImageFusion::packTaskStaticStatus(FusionTaskStaticResult &res, const string task_id, TaskPackStruct &tmp) {
    FusionStruct* out_res = (FusionStruct*)tmp.output;
    if(out_res->status <= 0) {
        return false;
    }

    res.task_id.assign(task_id);

    FusionArgs* param = (FusionArgs*) tmp.input;
    res.input.band.assign(param->band.begin(), param->band.end());
    res.input.idalg = param->idalg;
    res.input.idinter = param->idinter;
    res.input.logurl = param->logurl;
    res.input.msurl = param->msurl;
    res.input.outurl = param->outurl;
    res.input.panurl = param->panurl;

    res.output.brcoorvalidLatitude = out_res->brcoorvalid_latitude;
    res.output.brcoorvalidLongitude = out_res->brcoorvalid_longitude;
    res.output.brcoorwholeLatitude = out_res->brcoorwhole_latitude;
    res.output.brcoorwholeLongitude = out_res->brcoorwhole_longitude;

    res.output.cnttimeuse = out_res->cnttimeuse;
    res.output.datumname.assign(out_res->datumname);
    res.output.producetime.assign(out_res->producetime);
    res.output.productFormat.assign(out_res->product_format);
    res.output.projcentralmeridian = out_res->projcentralmeridian;
    res.output.projectioncode.assign(out_res->projectioncode);
    res.output.projectiontype.assign(out_res->projectiontype);
    res.output.projectionunits.assign(out_res->projectionunits);
    res.output.resolution = out_res->resolution;
    res.output.status = out_res->status;
    res.output.ulcoorvalidLatitude = out_res->ulcoorvalid_latitude;
    res.output.ulcoorvalidLongitude = out_res->ulcoorvalid_longitude;
    res.output.ulcoorwholeLatitude = out_res->ulcoorwhole_latitude;
    res.output.ulcoorwholeLongitude = out_res->ulcoorwhole_longitude;

    return true;
}

FusionInf ImageFusion::fuseSyn(const DirArgs& mapArg, const Ice::Current &) {

    string task_id = mapArg.at("id");
    if(m_finishMap.count(task_id) != 0) {
        return m_finishMap[task_id].output;
    }

    FusionInf obj;
    FusionArgs args;
    log_InputParameters(mapArg);
    bool flag = checkFusionArgv(mapArg, args);
    if(flag == false) {
        obj.status = ARGERROR;
        Log::Error("fuseSyn ## Image Fusion Parameters Error !");
        return obj;
    }
    FusionStruct* test = NULL;
    void* tmp = NULL;
    tmp = fusionInterface((void*)(&args));
    if(tmp == NULL) {
        Log::Error("Fetch Fusion Result Struct Failed !");
        return obj;
    }
    test = (FusionStruct*)tmp;
    deepCopyTask2RpcResult(*test, obj);
    delete test;
    log_OutputResult(obj);
    fillFinishTaskMap(task_id, args, obj);
    return obj;
}

int ImageFusion::fuseAsyn(const DirArgs& mapArgs, const Ice::Current&) {

    string task_id = mapArgs.at("id");
    if(m_finishMap.count(task_id) != 0) {
        return 1;
    }

    FusionArgs* args = new(std::nothrow) FusionArgs;
    if(args == NULL) {
        Log::Error("fuseAysn ## new FusionArgs Failed !");
        return -1;
    }

    log_InputParameters(mapArgs);
    bool flag = checkFusionArgv(mapArgs, *args);
    if(flag == false) {
        delete args;
        Log::Error("fuseAsyn ## Image Fusion Parameters Error !");
        return -1; // push task error
    }

    Task* task = new(std::nothrow) Task(&fusionInterface, (void*)args);
    if(task == NULL) {
        delete args;
        Log::Error("fuseAsyc ## new Task Failed !");
        return -1;
    }

    task->setTaskID(task_id);
    if(p_threadPool->add_task(task, task_id) != 0) {
        Log::Error("fuseAsyn ## thread Pool add Task Failed !");
        delete args;
        delete task;
        return -1; // Means For Add Task Failed !
    }
    return 1; // Means For Add Task Success !
}

string ImageFusion::askProcess(const DirArgs& mapArg, const Ice::Current&) {
    log_InputParameters(mapArg);
    return "";
}

FusionInf ImageFusion::fetchFuseRes(const DirArgs& mapArg, const Ice::Current&) {
    log_InputParameters(mapArg);
    FusionInf obj;
    if(mapArg.count("id") == 0) {
        obj.status = -1;
        Log::Error("fetchFuseRes ## Input Parameter InValid !");
        return obj;
    }
    string task_id = mapArg.at("id");
    if(m_finishMap.count(task_id) != 0) {
        return m_finishMap[task_id].output;
    }

    TaskPackStruct tmp;
    bool flag = p_threadPool->fetchResultByTaskID(task_id, tmp);
    if(flag == false) {
        obj.status = -1;
        Log::Error("fetchFuseRes ## fetch task id %s result Failed !", task_id.c_str());
    } else {
        deepCopyTask2RpcResult(*(FusionStruct*)(tmp.output), obj);
        log_OutputResult(obj);
        FusionTaskStaticResult res;
        flag = packTaskStaticStatus(res, task_id, tmp);
        delete (FusionArgs*)tmp.input;
        delete (FusionStruct*)tmp.output;
        if(flag == true) {
            m_finishMap[task_id] = res;
        }
    }
    return obj;
}

bool ImageFusion::checkFusionArgv(DirArgs mapArgs, FusionArgs &args) {
    if(mapArgs.count("id") == 0) {
        cerr << "Arg Map Configure Task ID Error !" << endl;
        return false;
    }

    if(mapArgs.count("panurl") == 0 || mapArgs.count("msurl") == 0 || mapArgs.count("outurl") == 0 || mapArgs.count("idalg") == 0 || mapArgs.count("idinter") == 0) {
        cerr << "Arg Map Configure Error !" << endl;
        return false;
    }

    if(mapArgs.count("band1") == 0 || mapArgs.count("band2") == 0 || mapArgs.count("band3") == 0) {
        cerr << "Arg Map Configure Bands Error !" << endl;
        return false;
    }

    ResultStatus status;
    checkFileInfo(mapArgs["panurl"], status);
    if(status.stauts != 0) {
        cerr << status.desc << endl;
        return false;
    }

    checkFileInfo(mapArgs["msurl"], status);
    if(status.stauts != 0) {
        cerr << status.desc << endl;
        return false;
    }

    int idalg, interpolation;
    int band[3] = {0,0,0};
    try {
        //FusionMethod
        idalg = stoi(mapArgs["idalg"]);
        if(idalg<1 || idalg>10)
            throw "Fusion Method does not exist";

        //Bandlist
        band[0] = stoi(mapArgs["band1"]);
        band[1] = stoi(mapArgs["band2"]);
        band[2] = stoi(mapArgs["band3"]);

        //InterpolationMethod
        interpolation = stoi(mapArgs["idinter"]);

        if(interpolation<1 || interpolation>5)
            throw "Interpolation Method does not exist";

        //compare the Fusion Args
        args.panurl = mapArgs["panurl"];
        args.msurl = mapArgs["msurl"];
        args.outurl = mapArgs["outurl"];
        //args.logurl = logPath;
        args.idalg = idalg;
        args.idinter = interpolation;
        args.band.assign(band, band+3);

    } catch (exception& e) {
        cerr << e.what() << endl;
        return false;
    } catch (const char * msg) {
        cerr << msg << endl;
        return false;
    }

    return true;
}
