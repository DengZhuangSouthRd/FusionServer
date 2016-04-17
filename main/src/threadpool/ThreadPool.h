#pragma once

#include <pthread.h>
#include <unistd.h>
#include <deque>
#include <iostream>
#include <fstream>
#include <vector>
#include <errno.h>
#include <string.h>
#include <string>
#include <map>

#include "Mutex.h"
#include "Task.h"

const int DEFAULT_POOL_SIZE = 10;
const int STARTED = 0;
const int STOPPED = 1;

using namespace std;

class ThreadPool {
public:
    ThreadPool();
    ThreadPool(int pool_size);
    ~ThreadPool();

public:
    void setPoolSize(const int pool_size);
    void setLogOfstream(ofstream& out);
    friend ostream& operator <<(ostream& out, string& info) {
        if(&out == NULL) {
            cout << info << endl;
            return cout;
        } else {
            out << info << endl;
            return out;
        }
    }

public:
    int initialize_threadpool();
    int destroy_threadpool();

public:
    int runningNumbers();
    int getPoolCapacity();
    void* execute_task(pthread_t thread_id);
    int add_task(Task* task, const string& task_id);
    bool is_aready_In_Map(const string& task_id);
    pthread_t getThreadIDByTaskID(const string& task_id);
    void join_task(const string& task_id);

private:
    int m_pool_size;
    volatile int m_pool_state;
    ofstream *m_pOutLog;

private:
    Mutex m_task_mutex;
    CondVar m_task_cond_var;

private:
    std::vector<pthread_t> m_threads;
    map<string, Task*> taskMap; // for <task_id, task*>
    map<string, pthread_t> threadMap; // for <task_id, pthread_t>
    std::deque<Task*> m_tasks;
};

