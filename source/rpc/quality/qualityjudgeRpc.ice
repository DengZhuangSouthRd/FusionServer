module RPCQualityJudge {

    sequence<double> DataArray;
    dictionary<string, DataArray> DatasMap;
    struct QualityInfo {
        int status;	//总分
        DatasMap imgsquality; //各指标的平均值
    };
    
    struct ImageParameter {
        string filePath;	//图像路径	必须
        int rowNum;			//图像行个数	不需要
        int colNum;			//图像列个数	不需要
        int bandNum;		//总共有多少波段	不需要
        int bitsPerPixel; 	// 8,16,24	固定值16
    };
    
    dictionary<string, ImageParameter> QualityMapArgs;
    
    struct QualityInputStruct {
        string id; // task id
        string algorithmkind; // quality algorithm class
        QualityMapArgs inputMap; // wait for judge parameters 
    };

    interface QualityInf {
        //提交同步任务，一直等待结果
        QualityInfo qualitySyn(QualityInputStruct inputArgs);

        //提交异步任务
        int qualityAsyn(QualityInputStruct inputArgs);//提交任务数据
        //k:id	v:		//任务唯一编号
        QualityInfo fetchQualityRes(string inputArgs);//取任务处理结果
    };
};

