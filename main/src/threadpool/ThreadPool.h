#pragma once

#include <pthread.h>
#include <unistd.h>
#include <deque>
#include <iostream>
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

typedef struct _PIDStatus{
    string desc;
    pthread_t pid;
    int status;
}PIDStatus;

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
    void* execute_thread();
    int add_task(Task* task);

private:
    int m_pool_size;
    Mutex m_task_mutex;
    CondVar m_task_cond_var;
    std::vector<pthread_t> m_threads;
    map<string,PIDStatus> hashMap;
    std::deque<Task*> m_tasks;
    volatile int m_pool_state;
};

