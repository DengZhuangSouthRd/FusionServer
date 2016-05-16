#ifndef UTILS_HPP
#define UTILS_HPP

#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <unistd.h>
#include <vector>
#include <exception>

#include "json/json-forwards.h"
#include "json/json.h"

using namespace std;
using namespace Json;

enum TASKSTATUS {
    TASKINIT,
    TASKRUNNING,
    TASKCOMPELETE,
    TASKERROR,
    ARGERROR
};

enum FileMode {
    EXISTS,
    WRITEABLE,
    READABLE,
    RWABLE,
    NOTEXISTS
};

typedef struct _ResultStatus {
    int stauts;
    FileMode mode;
    string desc;
}ResultStatus;

typedef struct _TaskPackStruct {
    void* input;
    void* output;
}TaskPackStruct;

void checkFileInfo(string filePath, ResultStatus& status);
bool isExistsFile(string filePath);

void read_config_Json(string fileName, map<string, string>& argvMap);


#endif // UTILS_HPP

