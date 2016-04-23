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
#include <set>

#include "Mutex.h"
#include "Task.h"

#include "../utils/log.h"

/*
 * This thread pool not manage the memory delete and free
 * Who Malloc and New, who should pay attention to the memory management !
*/

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

public:
    int initialize_threadpool();
    int destroy_threadpool();

public:
    int runningNumbers();
    int getPoolCapacity();
    void* execute_task(pthread_t thread_id);
    int add_task(Task* task, const string& task_id);
    bool is_aready_In_Map(const string& task_id);

private:
    int m_pool_size;
    volatile int m_pool_state;
    ofstream *m_pOutLog;

private:
    Mutex m_task_mutex;
    CondVar m_task_cond_var;

private:
    std::vector<pthread_t> m_threads;
    std::set<pthread_t> m_run_threads;
    map<string, Task*> taskMap; // for <task_id, task*>
    std::deque<Task*> m_tasks;
};

