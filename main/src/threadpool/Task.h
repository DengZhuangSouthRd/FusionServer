#pragma once

#include <pthread.h>
#include <unistd.h>
#include <deque>
#include <iostream>
#include <vector>
#include <errno.h>
#include <string.h>
#include <string>

using namespace std;

enum TASKSTATUS {
    TASKINIT,
    TASKRUNNING,
    TASKCOMPELETE,
    TASKERROR
};

class Task {
public:
    Task(void (*fn_ptr)(void*), void* arg); 
    ~Task();

public:
    void run();

public:
    void setTaskID(string task_id);
    const string getTaskID();
    pthread_t getThreadID();
    TASKSTATUS getTaskStatus();

private:
    void (*m_fn_ptr)(void*);
    void* m_arg;

private:
    string m_task_id;
    TASKSTATUS m_status;
};
