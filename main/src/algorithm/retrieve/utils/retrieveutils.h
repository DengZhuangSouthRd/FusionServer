#ifndef RETRIEVEUTILS_H
#define RETRIEVEUTILS_H

#include <iostream>
#include <fstream>
#include <string>
#include <map>

using namespace std;

string getPGConfInfo(const map<string, string>& argvMap);
void* retrieveInterface(void* args);

#endif // RETRIEVEUTILS_H
