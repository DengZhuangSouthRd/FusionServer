//
// Created by 贵阳 on 5/26/16.
//
#include "../utils/qualityutils.h"

double mainComprehensiveEvaluate(vector<double> resultvalue) {
    if(resultvalue.size() != 5) {
        cerr << "Result Value Input Parameter Length Error !" << endl;
        return -1;
    }
    double threshold[5][3] = {
            {40,35,30}, //
            {85,75,65},//
            {4,2,1},//
            {3,2.5,2.0},//
            {60,25,10}//
    };
    double weight[5] = {1.2, 1.2, 1.0, 0.8, 0.8};
    double deduct[4] = {0, 1.0, 12.0, 42.0};
    double score = 100.0;
    for(int i=0;i<resultvalue.size();i++) {
        if(resultvalue[i] >= threshold[i][0]) {
            score -= weight[i]*deduct[0];
        } else if(resultvalue[i] >= threshold[i][1]) {
            score -= weight[i]*deduct[1];
        } else if(resultvalue[i] >= threshold[i][2]) {
            score -= weight[i]*deduct[2];
        } else {
            score -= weight[i]*deduct[3];
        }
    }
    if(score<0.000001)
        score=0.0;
    return score;
}
