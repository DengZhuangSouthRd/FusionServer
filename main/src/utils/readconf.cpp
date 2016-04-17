/*************************************************************************
	> File Name: readconf.cpp
	> Author: 
	> Mail: 
	> Created Time: 2016年04月15日 星期五 15时16分54秒
 ************************************************************************/

#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <fstream>
#include <unistd.h>
#include <stdlib.h>

using namespace std;

void read_config(string fileName, map<string, string>& argvMap) {
    if(access(fileName.c_str(), 0) == -1) {
        cerr << "PG DataBase Configure File Not Exists !" << endl;
        exit(1);
    }
    ifstream in(fileName.c_str());
    if(!in.is_open()) {
        cerr << "PG DataBase Configure File Not Exists !" << endl;
        exit(1);
    }
    while(!in.eof()) {
        char buffer[512] = {'\0'};
        in.getline(buffer, 511);
        string info(buffer);
        size_t pos = info.find(':');
        if(pos == string::npos) {
            return ;
        }
        string keyVal = info.substr(0, pos);
        string valVal = info.substr(pos+1);
        argvMap[keyVal] = valVal;
    }
}

/*
int main() {
    map<string, string> argvMap;
    read_config("./conf/configure", argvMap);
    return 0;
}
*/
