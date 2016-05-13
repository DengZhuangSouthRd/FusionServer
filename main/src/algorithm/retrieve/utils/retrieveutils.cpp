#include "retrieveutils.h"

string getPGConfInfo(const map<string, string> &argvMap) {
    string str = "";
    str = "dbname=" + argvMap.at("RETRIEVEPG_NAME") + " user=" + argvMap.at("RETRIEVEPG_USER") + " password=" + argvMap.at("RETRIEVEPG_PASSWD") + " host=" + argvMap.at("RETRIEVEPG_HOST") + " port=" + argvMap.at("RETRIEVEPG_PORT");
    return str;
}

void* retrieveInterface(void *args) {
    string imgurl = *(string*)args;
    cout << imgurl << endl;
}
