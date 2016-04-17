/********************************************************************
	created:	2015/12/24
	created:	24:12:2015   14:31
	filename: 	D:\code\vs2010\C\Fusion\PCAFusion.h
	file path:	D:\code\vs2010\C\Fusion
	file base:	PCAFusion
	file ext:	h
	author:		YS
	
	purpose:	PCAFusion类声明
				主成分分析（PCA）融合算法
*********************************************************************/
#ifndef _PCAFusion_H_
#define _PCAFusion_H_

#include "GdalInf.h"
#include "Tools.h"
#include "PCA.h"


#ifndef DATA2D
#define DATA2D(Matrix,m,n,SizeCol) Matrix[(m)*(SizeCol)+(n)]
#endif

class PCAFusion: public Tools,public PCA{

public:
	//通过HPF算法实现PAN影像与MS影像融合
	void Local_MaxMin_PCA_Fusion(const char* Input_PAN_FileName, const char* Input_MS_FileName, const char* Output_MS_FileName,int InterpolationMethod);//局部融合
	
	void Global_MaxMin_PCA_Fusion(const char* Input_PAN_FileName, const char* Input_MS_FileName, const char* Output_MS_FileName,int InterpolationMethod);      //全局融合
	
	void Local_MeanStd_PCA_Fusion(const char* Input_PAN_FileName, const char* Input_MS_FileName, const char* Output_MS_FileName,int InterpolationMethod);//局部融合
	
	//版本1
	void MeanStd_PCA_Fusion(const char* Input_PAN_FileName, const char* Input_MS_FileName, const char* Output_MS_FileName,const char* LogName,int* bandlist,int InterpolationMethod);

};


#endif