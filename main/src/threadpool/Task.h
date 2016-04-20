#pragma once

#include <pthread.h>
#include <unistd.h>
#include <deque>
#include <iostream>
#include <vector>
#include <errno.h>
#include <string.h>
#include <string>

#include "../algorithm/fusion/PgInf.h"
#include "../utils/utils.h"

using namespace std;

class Task {
public:
    Task(void* (*fn_ptr)(void*), void* arg);
    ~Task();

public:
    void run();

public:
    void setTaskID(string task_id);
    const string getTaskID();
    pthread_t getThreadID();
    TASKSTATUS getTaskStatus();
    void* getTaskResult();

private:
    void* (*m_fn_ptr)(void*);
    void* m_arg;
    void* m_result;

private:
    string m_task_id;
    TASKSTATUS m_status;

};
