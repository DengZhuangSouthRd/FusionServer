#include "Fusion.h"


FusionInf fusion(string PanUrl,string MsUrl,string OutUrl,string LogUrl,int idalg,int* band,int interpolation) {
    /*
    *融合接口：fusion
    *PanUrl            全色图像或SAR图像存放路径
    *MsUrl             多光谱或高光谱图像存放路径
    *OutUrl			   融合结果存放路径
    *LogUrl            日志文件存放路径
    *idalg			   选择的融合算法
    *band              多光谱或高光谱参与融合的波段顺序
    *interpolation     插值方法
    *author:YS
    *time:2016.02.26
    */

    time_t now;
    struct tm *timenow;
    time(&now);
    timenow = localtime(&now);
    cout<<"fusion start time:"<<asctime(timenow)<<endl;
    time_t start= mktime(timenow);

    switch (idalg) {
    case 1: //BroveyFusion
        BroveyFusion brovey;
        brovey.Brovey_Fusion(PanUrl.c_str(),MsUrl.c_str(),OutUrl.c_str(),LogUrl.c_str(),band,interpolation);
        break;
    case 2: //HISFusion
        HSIFusion hsi;
        hsi.MeanStd_HSI_Fusion(PanUrl.c_str(),MsUrl.c_str(),OutUrl.c_str(),LogUrl.c_str(),band,interpolation);
        break;
    case 3: //PCAFusion
        PCAFusion pca;
        pca.MeanStd_PCA_Fusion(PanUrl.c_str(),MsUrl.c_str(),OutUrl.c_str(),LogUrl.c_str(),band,interpolation);
        break;
    case 4: //DWTFusion
        DWTFusion dwt;
        dwt.MeanStd_DWT_HIS_Fusion(PanUrl.c_str(),MsUrl.c_str(),OutUrl.c_str(),LogUrl.c_str(),band,interpolation);
        break;
    case 5: //Curvelet与HIS
        CurveletFusion curvelet_his;

        curvelet_his.MeanStd_Curvelet_HIS_Fusion(PanUrl.c_str(),MsUrl.c_str(),OutUrl.c_str(),LogUrl.c_str(),band,interpolation);
        break;
    case 6://GramSchmidtFusion
        GramSchmidtFusion GramSchmidt;
        GramSchmidt.GramSchmidt_Fusion(PanUrl.c_str(),MsUrl.c_str(),OutUrl.c_str(),LogUrl.c_str(),band,interpolation);
        break;
    case 7://HCS
        HCSFusion hcs;
        hcs.MeanStd_HCS_Fusion(PanUrl.c_str(),MsUrl.c_str(),OutUrl.c_str(),LogUrl.c_str(),band,interpolation);
        break;
    case 8://Curvelet与HCS
        CurveletFusion curvelet_hcs;
        curvelet_hcs.MeanStd_Curvelet_HCS_Fusion(PanUrl.c_str(),MsUrl.c_str(),OutUrl.c_str(),LogUrl.c_str(),band,interpolation);
        break;
    case 9://Curvelet与GramSchmidt
        CurveletFusion curvelet_gs;
        curvelet_gs.MeanStd_Curvelet_GramSchmidt_Fusion(PanUrl.c_str(),MsUrl.c_str(),OutUrl.c_str(),LogUrl.c_str(),band,interpolation);
        break;
    case 10://Curvelet与HCS
        CurveletFusion curvelet;
        curvelet.MeanStd_Curvelet_HCS_Fusion_New(PanUrl.c_str(),MsUrl.c_str(),OutUrl.c_str(),LogUrl.c_str(),band,interpolation);
        break;

    default: //error
        break;
    }
    time(&now);
    timenow = localtime(&now);
    string Producetime  = asctime(timenow); //融合产品生产时间

    cout<<"fusion end time:"<<Producetime<<endl;

    time_t end= mktime(timenow);
    timenow  = NULL;
    double Cnttimeuse = difftime(end,start);//融合算法所用时间
    cout<<"running time:"<<Cnttimeuse<<"s"<<endl;

    PgInf pginf;
    pginf.ReadInfToDB(Cnttimeuse,OutUrl,Producetime);//待写入数据库的相关信息

    return pginf.GetInf();
}
