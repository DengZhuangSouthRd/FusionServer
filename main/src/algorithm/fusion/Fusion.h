#include <time.h>

#include "BroveyFusion.h"
#include "HSIFusion.h"
#include "HCSFusion.h"
#include "PCAFusion.h"
#include "DWTFusion.h"
#include "CurveletFusion.h"
#include "GramSchmidtFusion.h"
#include "PgInf.h"


FusionInf fusion(string PanUrl,string MsUrl,string OutUrl,string LogUrl,int idalg,int* band,int interpolation);
