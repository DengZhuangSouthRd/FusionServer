#include "ThreadPool.h"

extern map<string, string> g_ConfMap;

ThreadPool* ThreadPool::p_ThreadPool = new ThreadPool();
ThreadPool* ThreadPool::getSingleInstance() {
    return p_ThreadPool;
}

void ThreadPool::revokeSingleInstance() {
    delete p_ThreadPool;
    p_ThreadPool = NULL;
}

ThreadPool::ThreadPool() : m_pool_size(DEFAULT_POOL_SIZE), m_task_size(DEFAULT_POOL_SIZE*1.5) {
    m_threads.clear();
    m_run_threads.clear();
    m_serializePath = g_ConfMap["SerializePath"];
    m_serializePathBak = g_ConfMap["SerializePathBak"];
    if(isExistsFile(m_serializePath) == false) {
        cerr << "Serialized File Does Not Exists !" << endl;
        Log::Error("Serialized File Does Not Exists !");
        throw runtime_error("Serialized File Does Not Exists !");
    }
    if(isExistsFile(m_serializePathBak) == false) {
        cerr << "Serialized Bak File Does Not Exists !" << endl;
        Log::Error("Serialized Bak File Does Not Exists !");
        throw runtime_error("Serialized Bak File Does Not Exists !");
    }
    getSerializeTaskResults();
    if(initialize_threadpool() != 0) {
        cerr << "Initialize ThreadPool failed !, Please Check and restart the Service !" << endl;
        Log::Error("Failed to initialize the Thread Pool !");
        throw runtime_error("Initialize ThreadPool failed !, Please Check and restart the Service !");
    }
}

ThreadPool::~ThreadPool() {
    serializeTaskResults();
    if (m_pool_state != STOPPED) {
        Log::Info("~ ThreadPool and Still Running !");
        cout << "~ ThreadPool and Still Running !" << endl;
        destroy_threadpool();
    }
}

void ThreadPool::setPoolSize(const int pool_size) {
    m_pool_size = pool_size;
    m_task_size = m_pool_size * 1.5;
}

void ThreadPool::setTaskUplimit(const int task_size) {
    m_task_size = task_size;
}

// We can't pass a member function to pthread_create.
// So created the wrapper function that calls the member function
// we want to run in the thread.
extern "C" {

void* start_thread(void* arg) {
    ThreadPool* tp = (ThreadPool*) arg;
    pthread_t tid = pthread_self();
    tp->execute_task(tid);
    return NULL;
}

}

int ThreadPool::initialize_threadpool() {
    m_pool_state = STARTED;
    int ret = -1;
    for(int i=0; i<m_pool_size; ++i) {
        pthread_t tid;
        ret = pthread_create(&tid, NULL, start_thread, (void*) this);
        if (ret != 0) {
            Log::Error("pthread_create() failed %d !", ret);
            cerr << "pthrad_create() failed " << ret << endl;
            return -1;
        }
        m_threads.push_back(tid);
    }
    return 0;
}

int ThreadPool::destroy_threadpool() {
    m_task_mutex.lock();
    m_pool_state = STOPPED;
    m_task_mutex.unlock();
    cout << "Broadcasting STOP signal to all threads..." << endl;
    Log::Info("Broadcasting STOP signal to all threads...");
    m_task_cond_var.broadcast(); // notify all threads we are shttung down

    for(set<pthread_t>::iterator it=m_run_threads.begin(); it!=m_run_threads.end();++it) {
        void * result;
        pthread_join(*it, &result);
        m_task_cond_var.broadcast(); // try waking up a bunch of threads that are still waiting
    }

    return 0;
}

int ThreadPool::runningNumbers() {
    return m_tasks.size();
}

int ThreadPool::getPoolCapacity() {
    return m_pool_size;
}

void* ThreadPool::execute_task(pthread_t thread_id) {
    Task* task = NULL;
    while(true) {
        // Try to pick a task
        m_task_mutex.lock();
        while((m_pool_state != STOPPED) && (m_tasks.empty())) {
            m_task_cond_var.wait(m_task_mutex.get_mutex_ptr());
        }

        // If the thread was woken up to notify process shutdown, return from here
        if(m_pool_state == STOPPED) {
            m_task_mutex.unlock();
            pthread_exit(NULL);
        }

        cout << "Residue the task numebr " << m_tasks.size() << endl;
        Log::Info("Residue the task numebr %d !", m_tasks.size());

        task = m_tasks.front();
        m_tasks.pop_front();
        if(m_run_threads.count(thread_id) == 0) {
            m_run_threads.insert(thread_id);
        } else {
            Log::Error("Thread id %d has already run !", thread_id);
            cerr << "Thread id " << thread_id << " has already run !" << endl;
        }
        m_task_mutex.unlock();

        task->run();
        string tmp_id = task->getTaskID();
        TaskStaticResult tmp_SaveTask;
        bool flag = task->packTaskStaticStatus(tmp_SaveTask);
        if(true == flag) {
            m_finishMap_mutex.lock();
                    m_finishMap[tmp_id] = tmp_SaveTask;
                    Log::Info("TaskID %s move to FinishMap!", tmp_id.c_str());
                    Log::Info("Finish Task size is %d !", m_finishMap.size());
                    if(m_finishMap.size() % 5 == 0) {
                        serializeTaskResults();
                    }
            m_finishMap_mutex.unlock();
        } else {
            Log::Error("TaskID %s RunStatus Failed !", tmp_id.c_str());
        }
        m_taskMap_mutex.lock();
            delete task;
            m_taskMap[tmp_id] = NULL;
            auto it = m_taskMap.find(tmp_id);
            m_taskMap.erase(it);
            Log::Info("TaskID %s have removed from TaskMap !", tmp_id.c_str());
        m_taskMap_mutex.unlock();

        m_run_threads.erase(thread_id);
    }
    return NULL;
}

int ThreadPool::add_task(Task* task, const string &task_id) {
    m_task_mutex.lock();
    task->setTaskID(task_id);
    m_tasks.push_back(task);
    Log::Info("Now the task size is %d !", m_tasks.size());
    m_taskMap[task_id] = task;
    // wake up one thread that is waiting for a task to be available
    m_task_cond_var.signal();
    m_task_mutex.unlock();
    return 0;
}

bool ThreadPool::fetchResultByTaskID(const string task_id, FusionInf &res) {
    // first step find in m_finishMap, if not in this Map
    // second step find in m_taskMap, search it process status

    if(m_finishMap.count(task_id) != 0) {
        deepCopyTaskResult(m_finishMap.at(task_id).output, res);
        return true;
    } else if(m_taskMap.count(task_id) != 0) {
        Log::Info("Fetch task id %s not finished !", task_id.c_str());
        return false;
    } else {
        Log::Error("Fetch task_id %s have not been push to this pool !", task_id.c_str());
        return false;
    }
    return false;
}

bool ThreadPool::isExistsFile(const string filePath) {
    if(access(filePath.c_str(), 0) == 0) {
        return true;
    }
    return false;
}

void ThreadPool::fillFinishTaskMap(const string &task_id, const FusionArgs &inParam, const FusionInf &outParam) {
    if(m_finishMap.count(task_id) == 0) {
        TaskStaticResult tmp;
        tmp.task_id.assign(task_id);
        deepCopyTaskInputParameter(inParam, tmp.input);
        deepCopyTaskResult(outParam, tmp.output);
        m_finishMap_mutex.lock();
            m_finishMap[task_id] = tmp;
            Log::Info("Finish Task size is %d !", m_finishMap.size());
            if(m_finishMap.size() % 5 == 0) {
                serializeTaskResults();
            }
        m_finishMap_mutex.unlock();
    }
}

// get the over task from the Json File
int ThreadPool::getSerializeTaskResults() {
    Json::Reader reader;
    Json::Value root;
    Json::Value::Members members;
    std::ifstream in;
    in.open(m_serializePath.c_str(), std::ios_base::binary);
    if(in.is_open() == false) {
        throw runtime_error("Open Serialize File Error !");
        cerr << "Open Seriazlize file Error !" << endl;
    }
    bool flag = reader.parse(in,root, false);
    if(flag == false) {
        throw runtime_error("Parse Serialize Json File failed !");
        cerr << "Parse Serialize Json File failed !" << endl;
    }
    members = root.getMemberNames();
    for(Json::Value::Members::iterator it=members.begin(); it!=members.end(); ++it) {
        std::string key = *it;
        Json::Value node = root[key];
        Json::Value inNode = node[0];
        Json::Value outNode = node[1];

        TaskStaticResult tmp;
        tmp.task_id.assign(key);
        tmp.input.panurl.assign(inNode["panurl"].asString());
        tmp.input.outurl.assign(inNode["outurl"].asString());
        tmp.input.msurl.assign(inNode["msurl"].asString());
        tmp.input.logurl.assign(inNode["logurl"].asString());
        tmp.input.idinter = inNode["idinter"].asInt();
        tmp.input.idalg = inNode["idalg"].asInt();
        tmp.input.band.push_back(inNode["band1"].asInt());
        tmp.input.band.push_back(inNode["band2"].asInt());
        tmp.input.band.push_back(inNode["band3"].asInt());

        tmp.output.brcoorvalidLatitude = outNode["brcoorvalidLatitude"].asDouble();
        tmp.output.brcoorvalidLongitude = outNode["brcoorvalidLongitude"].asDouble();
        tmp.output.brcoorwholeLatitude = outNode["brcoorwholeLatitude"].asDouble();
        tmp.output.brcoorwholeLongitude = outNode["brcoorwholeLongitude"].asDouble();
        tmp.output.cnttimeuse = outNode["cnttimeuse"].asDouble();
        tmp.output.datumname.assign(outNode["datumname"].asString());
        tmp.output.producetime.assign(outNode["producetime"].asString());
        tmp.output.productFormat.assign(outNode["productFormat"].asString());
        tmp.output.projcentralmeridian = outNode["projcentralmeridian"].asFloat();
        tmp.output.projectioncode.assign(outNode["projectioncode"].asString());
        tmp.output.projectiontype.assign(outNode["projectiontype"].asString());
        tmp.output.projectionunits.assign(outNode["projectionunits"].asString());
        tmp.output.resolution = outNode["resolution"].asFloat();
        tmp.output.status = outNode["status"].asInt();
        tmp.output.ulcoorvalidLatitude = outNode["ulcoorvalidLatitude"].asDouble();
        tmp.output.ulcoorvalidLongitude = outNode["ulcoorvalidLongitude"].asDouble();
        tmp.output.ulcoorwholeLatitude = outNode["ulcoorwholeLatitude"].asDouble();
        tmp.output.ulcoorwholeLongitude = outNode["ulcoorwholeLongitude"].asDouble();
        m_finishMap[key] = tmp;
    }
    in.close();
    Log::Info("The Task Pool Have %d tasks !", members.size());
    return members.size();
}

//fetch all task id and task result to serialize the completed task !
int ThreadPool::serializeTaskResults() {

    Json::FastWriter writer;
    Json::Value root;

    for(map<string, TaskStaticResult>::iterator it=m_finishMap.begin(); it!=m_finishMap.end(); ++it) {
        string key = it->first;
        TaskStaticResult res = it->second;
        Json::Value input;
        input["panurl"] = res.input.panurl;
        input["outurl"] = res.input.outurl;
        input["msurl"] = res.input.msurl;
        input["logurl"] = res.input.logurl;
        input["idinter"] = res.input.idinter;
        input["idalg"] = res.input.idalg;
        input["band1"] = res.input.band[0];
        input["band2"] = res.input.band[1];
        input["band3"] = res.input.band[2];

        Json::Value outres;
        outres["brcoorvalidLatitude"] = res.output.brcoorvalidLatitude;
        outres["brcoorvalidLongitude"] = res.output.brcoorvalidLongitude;
        outres["brcoorwholeLatitude"] = res.output.brcoorwholeLatitude;
        outres["brcoorwholeLongitude"] = res.output.brcoorwholeLongitude;
        outres["cnttimeuse"] = res.output.cnttimeuse;
        outres["datumname"] = res.output.datumname;
        outres["producetime"] = res.output.producetime;
        outres["productFormat"] = res.output.productFormat;
        outres["projcentralmeridian"] = res.output.projcentralmeridian;
        outres["projectioncode"] = res.output.projectioncode;
        outres["projectiontype"] = res.output.projectiontype;
        outres["projectionunits"] = res.output.projectionunits;
        outres["resolution"] = res.output.resolution;
        outres["status"] = res.output.status;
        outres["ulcoorvalidLatitude"] = res.output.ulcoorvalidLatitude;
        outres["ulcoorvalidLongitude"] = res.output.ulcoorvalidLongitude;
        outres["ulcoorwholeLatitude"] = res.output.ulcoorwholeLatitude;
        outres["ulcoorwholeLongitude"] = res.output.ulcoorwholeLongitude;

        root[key].append(input);
        root[key].append(outres);
    }
    std::string strRoot = writer.write(root);

    std::ofstream out;
    out.open(m_serializePathBak.c_str(), std::ios_base::binary);
    if(out.is_open() == false) {
        throw runtime_error("Open Serialize Bak File Error !");
        cerr << "Open Seriazlize Bak file Error !" << endl;
    }
    out << strRoot;
    out.close();


    out.open(m_serializePath.c_str(), std::ios_base::binary);
    if(out.is_open() == false) {
        throw runtime_error("Open Serialize File Error !");
        cerr << "Open Seriazlize file Error !" << endl;
    }
    out << strRoot;
    out.close();

    return m_finishMap.size();
}
