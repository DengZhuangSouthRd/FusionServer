#include "Server.h"

map<string, string> g_ConfMap;

Server::Server() {
    status = 0;
    Log::Initialise(g_ConfMap["LOGPATH"]);
    Log::SetThreshold(Log::LOG_TYPE_INFO);
    PUSH_LOG_STACK;
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
        Ice::ObjectPtr object = new ImageFusion();
        adapter->add(object,ic->stringToIdentity(g_ConfMap["FUSIONIDENTITY"]));
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
