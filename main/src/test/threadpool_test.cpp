#include <iostream>
#include <limits.h>
#include <unistd.h>
#include <stdio.h>

#include "ThreadPool.h"

using namespace std;

const int MAX_TASKS = 2;

void hello(void* arg) {
    int* x = (int*) arg;
    double num=0;
    for(int i=INT_MIN;i<INT_MAX;++i) {
        num++;
    }
    cout << "#$##$#$#$" << num << endl;
    sleep(5);
}

int main(int argc, char* argv[]) {

    ThreadPool tp(2);
    int ret = tp.initialize_threadpool();
    if(ret == -1) {
        cerr << "Failed to initialize thread pool!" << endl;
        return 0;
    }

    for(int i = 0; i < MAX_TASKS; i++) {
        Task* t = new Task(&hello, (void*) &i);
        tp.add_task(t);
    }

    //tp.destroy_threadpool();
    cout << "Exiting app..." << endl;

    return 0;
}
