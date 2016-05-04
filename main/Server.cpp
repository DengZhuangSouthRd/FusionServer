#include "Server.h"
#include <string>

using namespace std;

map<string, string> g_ConfMap;

ImageFusion::ImageFusion() {
    m_threadPool.setPoolSize(10);
    int ret = m_threadPool.initialize_threadpool();
    if(ret == -1) {
        Log::Error("Failed to initialize the Thread Pool !");
        throw bad_alloc();
    }
    m_logPath = g_ConfMap["LOGPATH"];

    Log::Initialise(m_logPath);
    Log::SetThreshold(Log::LOG_TYPE_INFO);
    PUSH_LOG_STACK;
}

ImageFusion::~ImageFusion() {

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

::RPCWiseFuse::FusionInf ImageFusion::fuseSyn(const DirArgs& mapArg, const Ice::Current &) {
    ::RPCWiseFuse::FusionInf obj;
    FusionArgs args;
    log_InputParameters(mapArg);
    bool flag = checkFusionArgv(mapArg, m_logPath, args);
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
    return obj;
}

int ImageFusion::fuseAsyn(const DirArgs& mapArgs, const Ice::Current&) {
    ::RPCWiseFuse::FusionInf obj;
    FusionArgs* args = new(std::nothrow) FusionArgs;
    if(args == NULL) {
        Log::Error("fuseAysn ## new FusionArgs Failed !");
        return -1;
    }
    log_InputParameters(mapArgs);
    bool flag = checkFusionArgv(mapArgs, m_logPath, *args);
    if(flag == false) {
        obj.status = ARGERROR;
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
    string task_id = mapArgs.at("id");
    task->setTaskID(task_id);
    if(m_threadPool.add_task(task, task_id) != 0) {
        Log::Error("fuseAsyn ## thread Pool add Task Failed !");
        delete args;
        delete task;
        return -1; // Means For Add Task Failed !
    }
    return 0; // Means For Add Task Success !
}

string ImageFusion::askProcess(const DirArgs& mapArg, const Ice::Current&) {
    log_InputParameters(mapArg);
    return "";
}

::RPCWiseFuse::FusionInf ImageFusion::fetchFuseRes(const DirArgs& mapArg, const Ice::Current&) {
    log_InputParameters(mapArg);
    ::RPCWiseFuse::FusionInf obj;
    if(mapArg.count("id") == 0) {
        obj.status = -1;
        Log::Error("fetchFuseRes ## Input Parameter InValid !");
        return obj;
    }
    string task_id = mapArg.at("id");
    bool flag = m_threadPool.fetchResultByTaskID(task_id, obj);
    if(flag == false) {
        obj.status = -1;
        Log::Error("fetchFuseRes ## fetch task id %s result Failed !", task_id.c_str());
    } else {
        log_OutputResult(obj);
    }
    return obj;
}

bool ImageFusion::checkFusionArgv(DirArgs mapArgs, string &logPath, FusionArgs &args) {
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

Server::Server() {
    status = 0;
}

Server::~Server() {
    close();
}

void Server::initRpc(int argc, char** argv, string conn) {
    connParam = conn;
    m_argc = argc;
    m_argv = argv;
    try {
        ic = Ice::initialize(argc,argv);
        adapter = ic->createObjectAdapterWithEndpoints("ImageRpcAdapter",connParam);
        Ice::ObjectPtr object = new ImageFusion;
        adapter->add(object,ic->stringToIdentity(g_ConfMap["IDENTITY"]));
        adapter->activate();
        ic->waitForShutdown();
    } catch (const Ice::Exception& e) {
        cerr << e << endl;
        status = 1;
    } catch (const char* msg) {
        cerr << msg << endl;
        status = 1;
    } catch (const exception& e) {
        cerr << e.what();
        status = 1;
    }
}

void Server::restart() {
    close();
    initRpc(m_argc, m_argv, connParam);
}

void Server::close() {
    if(ic) {
        try {
            ic->destroy();
        } catch (const Ice::Exception& e) {
            cerr << e << endl;
            status = 1;
        }
    }
}

int main(int argc,char* argv[]) {
    if(argc != 2) {
        cerr << "Argc and Argv Format Error !" << endl;
        cerr << "Please Enter the configure.json file path !" << endl;
        exit(1);
    }
    string configPath(argv[1]);
    try {
        read_config_Json(configPath, g_ConfMap);
    } catch (runtime_error &err) {
        cerr << err.what() << endl;
        exit(1);
    }

    string cmd = "default -h " + g_ConfMap["SERVERIP"] + " -p " + g_ConfMap["PORT"];
    Server obj_server;
    obj_server.initRpc(argc, argv, cmd);

    return 0;
}
