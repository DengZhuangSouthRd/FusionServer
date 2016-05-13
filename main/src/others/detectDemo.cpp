#include <iostream>
#include <vector>
#include <string>
#include "SR.h"

#include <algorithm>
using namespace std;

int main() {
	vector<double> y;
	double min_residual = 0.0000001;
	int sparsity = 2;

	y.push_back(0.1189);
	y.push_back(0.1234);
	
	vector<string> filepath;
	filepath.push_back("1.csv");
	filepath.push_back("2.csv");

	SR<double> srclassify(filepath);
	cout << srclassify.SRClassify(y, min_residual, sparsity) << endl;

	return 0;
}