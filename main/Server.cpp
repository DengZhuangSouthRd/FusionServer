#include "Server.h"
#include <string>

using namespace std;

ImageFusion::ImageFusion() {
    m_threadPool.setPoolSize(50);
    m_logPath = "./data/log/loginfo.log";
}

ImageFusion::~ImageFusion() {

}

void ImageFusion::setLogPath(string logPath) {
    m_logPath = logPath;
}

FusionInf ImageFusion::fuseSyn(const DirArgs& mapArg, const Ice::Current &) {
    FusionInf obj;
    FusionArgs args;
    bool flag = checkFusionArgv(mapArg, m_logPath, args);
    if(flag == false) {
        obj.status = ARGERROR;
    }
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

FusionInf ImageFusion::fetchFuseRes(const DirArgs& mapArg, const Ice::Current&) {
    FusionInf obj;
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

    checkFileInfo(mapArgs["outurl"], status);
    if(status.stauts != -1) {
        cerr << status.desc << endl;
        return false;
    }

    checkFileInfo(logPath, status);
    if(status.stauts == -1 || (status.stauts == 0 && (status.mode == WRITEABLE || status.mode == RWABLE))) {
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
            args.logurl = logPath;
            args.idalg = idalg;
            args.idinter = interpolation;
            args.band.assign(band, band+3);

        } catch (exception& e) {
            cerr << e.what() << endl;
            return false;
        }
    } else {
        cerr << "Log File Has Not Write Permission" << endl;
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
        adapter->add(object,ic->stringToIdentity("ImageRpc"));
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
    obj_server.initRpc(argc, argv, "default -h 10.2.3.119 -p 9999");
    return 0;
}
