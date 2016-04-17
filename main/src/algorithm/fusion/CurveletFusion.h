/********************************************************************
	created:	2016/01/20
	created:	20:1:2016   18:06
	filename: 	D:\code\vs2010\C\Fusion\CurveletFusion.h
	file path:	D:\code\vs2010\C\Fusion
	file base:	CurveletFusion
	file ext:	h
	author:		YS
	
	purpose:	CurveletFusion类声明
				Curvelet与HIS结合的融合算法
				适用于三个波段
*********************************************************************/
#ifndef _CurveletFusion_H_
#define _CurveletFusion_H_

#include "GdalInf.h"
#include "curvelet/nummat.hpp"
#include "Tools.h"
#include "HSI.h"
#include "HCS.h"
#include "GramSchmidtFusion.h"

using namespace fdct_wrapping_ns;

#ifndef DATA2D
#define DATA2D(Matrix,m,n,SizeCol) Matrix[(m)*(SizeCol)+(n)]
#endif

class CurveletFusion: public HSI,public HCS,public GramSchmidtFusion{ //GramSchmidtFusion:public Tools

public:
	//通过Curvelet算法实现PAN影像与MS影像融合
	void Local_MaxMin_Curvelet_HIS_Fusion(const char* Input_PAN_FileName, const char* Input_MS_FileName, const char* Output_MS_FileName,int InterpolationMethod);
	void Local_MeanStd_Curvelet_HIS_Fusion(const char* Input_PAN_FileName, const char* Input_MS_FileName, const char* Output_MS_FileName,int InterpolationMethod);

	void Global_MaxMin_Curvelet_HIS_Fusion(const char* Input_PAN_FileName, const char* Input_MS_FileName, const char* Output_MS_FileName,int InterpolationMethod);
	void Global_MeanStd_Curvelet_HIS_Fusion(const char* Input_PAN_FileName, const char* Input_MS_FileName, const char* Output_MS_FileName,int InterpolationMethod);

	//版本1
	void MeanStd_Curvelet_HIS_Fusion(const char* Input_PAN_FileName, const char* Input_MS_FileName, const char* Output_MS_FileName,const char* LogName,int* bandlist,int InterpolationMethod);
	//2016.3.7
	void MeanStd_Curvelet_HCS_Fusion(const char* Input_PAN_FileName, const char* Input_MS_FileName, const char* Output_MS_FileName,const char* LogName,int* bandlist,int InterpolationMethod);
	//2016.3.8
	void MeanStd_Curvelet_GramSchmidt_Fusion(const char* Input_PAN_FileName, const char* Input_MS_FileName, const char* Output_MS_FileName,const char* LogName,int* bandlist,int InterpolationMethod);
	

	void MeanStd_Curvelet_HCS_Fusion_New(const char* Input_PAN_FileName, const char* Input_MS_FileName, const char* Output_MS_FileName,const char* LogName,int* bandlist,int InterpolationMethod);

	//计算局部均值，标准差及方向信息熵
	void LocalMeanStdEntropy(CpxNumMat Mat,int i,int j,int localSize,float & Mean,float &Std,float &Entropy){
		int p,q;
		int height= Mat.m();
		int width = Mat.n();
		int tmp1,tmp2;
		Mean = 0;
		Std = 0;
		Entropy = 0;
		for (p=-localSize/2; p<localSize/2;p++)
			for (q=-localSize/2; q<localSize/2;q++){
				tmp1 = i+p;
				tmp2 = j+q;
				if(tmp1 <0)
					tmp1 = 0;
				if(tmp1 >=height)
					tmp1 = height-1;
				if(tmp2 <0)
					tmp2 = 0;
				if(tmp2 >=width)
					tmp2 = height-1;
				Mean += abs(Mat(tmp1,tmp2));
				if (abs(Mat(tmp1,tmp2)) !=0)
					Entropy += (pow(abs(Mat(tmp1,tmp2)),2)*log10(pow(abs(Mat(tmp1,tmp2)),2))/log10(2.0));
			}

			Mean = Mean /(localSize*localSize);

			Entropy = Entropy /(localSize*localSize);
			
			for (p=-localSize/2; p<localSize/2;p++)
				for (q=-localSize/2; q<localSize/2;q++){
					tmp1 = i+p;
					tmp2 = j+q;
					if(tmp1 <0)
						tmp1 = 0;
					if(tmp1 >=height)
						tmp1 = height-1;
					if(tmp2 <0)
						tmp2 = 0;
					if(tmp2 >=width)
						tmp2 = height-1;
					Std += pow(abs(Mat(tmp1,tmp2))-Mean,2);
				}

			Std = sqrt(Std/(localSize*localSize));

	}
};


#endif
