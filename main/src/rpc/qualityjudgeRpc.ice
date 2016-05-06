module RPCQualityJudge {

    sequence<double> DataArray;
    dictionary<string, DataArray> DatasMap;
    struct QualityInfo {
        int status;
        DatasMap imgsquality;
    };
    
    struct ImageParameter {
        string filePath;
        int rowNum;
        int colNum;
        int bandNum;
        int bitsPerPixel; // 8,16,24
    };
    
    dictionary<string, ImageParameter> QualityMapArgs;
    
    struct QualityInputStruct {
        string id; // task id
        int algorithmkind; // quality algorithm class
        QualityMapArgs inputMap; // wait for judge parameters 
    };

    interface QualityInf {
        //提交同步任务，一直等待结果
        QualityInfo qualitySyn(QualityInputStruct inputArgs);

        //提交异步任务
        int qualityAsyn(QualityInputStruct inputArgs);//提交任务数据
        //k:id	v:		//任务唯一编号
        QualityInfo fetchQualityRes(QualityInputStruct inputArgs);//取任务处理结果
    };
};

