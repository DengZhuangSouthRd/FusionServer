#ifndef FUSIONUTILS_H
#define FUSIONUTILS_H

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <map>
#include <unistd.h>
#include <vector>
#include <exception>
#include <time.h>

#include "../BroveyFusion.h"
#include "../HSIFusion.h"
#include "../HCSFusion.h"
#include "../PCAFusion.h"
#include "../DWTFusion.h"
#include "../CurveletFusion.h"
#include "../GramSchmidtFusion.h"
#include "../PgInf.h"

#include "../../../rpc/wisefuseRpc.h"
#include "../../../utils/json/json-forwards.h"
#include "../../../utils/json/json.h"

using namespace std;
using namespace RPCWiseFuse;
using namespace Json;

typedef struct _FusionArgs {
    string panurl;
    string msurl;
    string outurl;
    string logurl;
    int idalg;
    vector<int> band;
    int idinter;
}FusionArgs;

void* fusionInterface(void* args);

void deepCopyTaskInputParameter(const FusionArgs& src, FusionArgs& dest);
void deepCopyTaskResult(const FusionInf &src, FusionInf &dest);
void deepCopyTask2RpcResult(const FusionStruct& src, FusionInf& dest);


FusionStruct* fusion(string PanUrl,string MsUrl,string OutUrl,string LogUrl,int idalg,int* band,int interpolation);

#endif // FUSIONUTILS_H
