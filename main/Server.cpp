#include "Server.h"
#include <string>

using namespace std;

ImageFusion::ImageFusion() {
    m_threadPool.setPoolSize(20);
    m_logPath = "/home/fighter/Documents/ImageFusion/main/data/log/loginfo.log";
    Log::Initialise(m_logPath);
    Log::SetThreshold(Log::LOG_TYPE_INFO);
    PUSH_LOG_STACK;
}

ImageFusion::~ImageFusion() {

}

void ImageFusion::updateStructInfo(FusionStruct srcInf, FusionInf &destInf) {
    destInf.brcoorvalidLatitude = srcInf.brcoorvalid_latitude;
    destInf.brcoorvalidLongitude = srcInf.brcoorvalid_longitude;
    destInf.brcoorwholeLatitude = srcInf.brcoorwhole_latitude;
    destInf.brcoorwholeLongitude = srcInf.brcoorwhole_longitude;

    destInf.cnttimeuse = srcInf.cnttimeuse;
    destInf.datumname = srcInf.datumname;

    destInf.producetime = srcInf.producetime;
    destInf.productFormat = srcInf.product_format;
    destInf.projcentralmeridian = srcInf.projcentralmeridian;
    destInf.projectioncode = srcInf.projectioncode;
    destInf.projectiontype = srcInf.projectiontype;
    destInf.projectionunits = srcInf.projectionunits;

    destInf.resolution = srcInf.resolution;
    destInf.status = srcInf.status;
    string str = "brcoorvalidLatitude = " + to_string(destInf.brcoorvalidLatitude)
            + " brcoorvalidLongitude = " + to_string(destInf.brcoorvalidLongitude)
            + " brcoorwholeLatitude = " + to_string(destInf.brcoorwholeLatitude)
            + " brcoorwholeLongitude = " + to_string(destInf.brcoorwholeLongitude)
            + " cnttimeuse = " + to_string(destInf.cnttimeuse)
            + " datumname = " + destInf.datumname
            + " producetime = " + destInf.producetime
            + " productFormat = " + destInf.productFormat
            + " projcentralmeridian = " + to_string(destInf.projcentralmeridian)
            + " projectioncode = " + destInf.projectioncode
            + " projectiontype = " + destInf.projectiontype
            + " projectionunits = " + destInf.projectionunits
            + " resolution = " + to_string(destInf.resolution);
    Log::Info(str);
}

::RPCWiseFuse::FusionInf ImageFusion::fuseSyn(const DirArgs& mapArg, const Ice::Current &) {
    ::RPCWiseFuse::FusionInf obj;
    FusionArgs args;
    bool flag = checkFusionArgv(mapArg, m_logPath, args);
    if(flag == false) {
        obj.status = ARGERROR;
        cerr << "Image Fusion Parameters Error !" << endl;
        return obj;
    }
    FusionStruct* test = NULL;
    void* tmp = NULL;
    tmp = fusionInterface((void*)(&args));
    if(tmp == NULL) {
        return obj;
    }
    test = (FusionStruct*)tmp;
    updateStructInfo(*test, obj);
    free(test);
    return obj;
}

int ImageFusion::fuseAsyn(const DirArgs& mapArg, const Ice::Current&) {
    FusionArgs args;
    bool flag = checkFusionArgv(mapArg, m_logPath, args);
    if(flag == false) {
        return -1; // push task error
    }
    return 0; // push task success
}

string ImageFusion::askProcess(const DirArgs& mapArg, const Ice::Current&) {
    return "";
}

::RPCWiseFuse::FusionInf ImageFusion::fetchFuseRes(const DirArgs& mapArg, const Ice::Current&) {
    ::RPCWiseFuse::FusionInf obj;
    if(mapArg.count("id") == 0) {

    }
    //string task_id = mapArg;
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
        adapter->add(object,ic->stringToIdentity("WISEFUSION"));
        adapter->activate();
        ic->waitForShutdown();
    } catch (const Ice::Exception& e) {
        cerr << e << endl;
        status = 1;
    } catch (const char* msg) {
        cerr << msg << endl;
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
    Server obj_server;
    obj_server.initRpc(argc, argv, "default -h 127.0.0.1 -p 9999");
    return 0;
}
