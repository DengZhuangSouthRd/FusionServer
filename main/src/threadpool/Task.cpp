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

bool Task::packTaskStaticStatus(TaskStaticResult &res) {
    FusionStruct* out_res = (FusionStruct*)m_result;
    if(out_res->status <= 0) {
        return false;
    }
    res.task_id = m_task_id;

    FusionArgs* param = (FusionArgs*) m_arg;
    res.input.band.assign(param->band.begin(), param->band.end());
    res.input.idalg = param->idalg;
    res.input.idinter = param->idinter;
    res.input.logurl = param->logurl;
    res.input.msurl = param->msurl;
    res.input.outurl = param->outurl;
    res.input.panurl = param->panurl;


    res.output.brcoorvalidLatitude = out_res->brcoorvalid_latitude;
    res.output.brcoorvalidLongitude = out_res->brcoorvalid_longitude;
    res.output.brcoorwholeLatitude = out_res->brcoorwhole_latitude;
    res.output.brcoorwholeLongitude = out_res->brcoorwhole_longitude;

    res.output.cnttimeuse = out_res->cnttimeuse;
    res.output.datumname.assign(out_res->datumname);
    res.output.producetime.assign(out_res->producetime);
    res.output.productFormat.assign(out_res->product_format);
    res.output.projcentralmeridian = out_res->projcentralmeridian;
    res.output.projectioncode.assign(out_res->projectioncode);
    res.output.projectiontype.assign(out_res->projectiontype);
    res.output.projectionunits.assign(out_res->projectionunits);
    res.output.resolution = out_res->resolution;
    res.output.status = out_res->status;
    res.output.ulcoorvalidLatitude = out_res->ulcoorvalid_latitude;
    res.output.ulcoorvalidLongitude = out_res->ulcoorvalid_longitude;
    res.output.ulcoorwholeLatitude = out_res->ulcoorwhole_latitude;
    res.output.ulcoorwholeLongitude = out_res->ulcoorwhole_longitude;

    return true;
}
