#include "Task.h"

Task::Task(void* (*fn_ptr)(void *), void* arg) {
    m_fn_ptr = fn_ptr;
    m_arg = arg;
    m_status = TASKINIT;
    m_result = NULL;
}

Task::~Task() {
    if(m_arg) {
        m_arg = (FusionArgs*)m_arg;
        delete m_arg;
    }
    if(m_result) {
        m_result = (FusionStruct*)m_result;
        delete m_result;
    }
}

void Task::run() {
    m_status = TASKRUNNING;
    m_result = (*m_fn_ptr)(m_arg);
    m_status = TASKCOMPELETE;
}

void Task::setTaskID(string task_id) {
    m_task_id = task_id;
}

const string Task::getTaskID() {
    return  m_task_id;
}

pthread_t Task::getThreadID() {
    return pthread_self();
}

TASKSTATUS Task::getTaskStatus() {
    return m_status;
}

bool Task::packTaskStaticStatus(TaskPackStruct& res) {
    res.input = m_arg;
    res.output = m_result;
}
