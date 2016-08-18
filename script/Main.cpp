#include "PrintToDB.h"
#include <iostream>
using namespace std;


int main(){
	string pgconfig = "dbname=postgres user=postgres password=postgres hostaddr=127.0.0.1 port=5432";
	PrintToDB print(pgconfig);
	
	string filepath = "Optical";
	string outpath = "wisefuse1";
	if (!print.PrintToOptical(filepath,outpath)){
		cerr << "Print Error."<< endl;
	}

	return 0;
}
