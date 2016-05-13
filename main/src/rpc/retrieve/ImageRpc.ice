/*
主要是用来设计海量图像检索的RPC调用接口
*/

module RPCImgRecong {
    struct WordWiki {
        string key; //关键字
        string abstr; //
        string descr; //描述信息
    };

    sequence<string> ListString;
    dictionary<string, string> DictStr2Str;

    struct ImgRes {
	    int status;
        ListString imgRemote;
        ListString imgPic;
    };
    struct WordRes {
	    int status;
        ListString keyWords;
    };
    
    interface ImgRetrieval {
        WordWiki wordGetKnowledge(string word); // 通过图像的名字，得到图像的知识库信息
        WordRes wordSearch(DictStr2Str mapArg); // 通过单词搜索得到单词对象列表
        ImgRes wordSearchImg(DictStr2Str mapArg); //通过单词搜索得到图像地址列表
        ImgRes imgSearch(DictStr2Str mapArg); //通过图像搜索得到图像地址列表
    };
};
