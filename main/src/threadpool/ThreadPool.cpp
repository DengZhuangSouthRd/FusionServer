#include "ThreadPool.h"

ThreadPool::ThreadPool() : m_pool_size(DEFAULT_POOL_SIZE), m_task_size(DEFAULT_POOL_SIZE*1.5) {
    m_threads.clear();
    m_run_threads.clear();
    m_serializePath = "/home/fighter/Documents/ImageFusion/main/data/serialize/task_backup.json";
    if(isExistsFile(m_serializePath) == false) {
        throw runtime_error("Serialized File Does Not Exists !");
        cerr << "Serialized File Does Not Exists !" << endl;
    }
}

ThreadPool::ThreadPool(int pool_size) : m_pool_size(pool_size), m_task_size(pool_size*1.5) {
    m_threads.clear();
    m_run_threads.clear();
    m_serializePath = "/home/fighter/Documents/ImageFusion/main/data/serialize/task_backup.json";
    if(isExistsFile(m_serializePath) == false) {
        throw runtime_error("Serialized File Does Not Exists !");
        cerr << "Serialized File Does Not Exists !" << endl;
    }
}

ThreadPool::~ThreadPool() {
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

//fetch all task id and task result to serialize the completed task !
int ThreadPool::serializeTaskResults() {
    //1. judge the file if exists
    //2. read the json file extract the task id into the memory
    //3. insert the new task info
    //4. write to the json file
    Json::Reader reader;
    Json::Value root;
    bool flag = reader.parse(m_serializePath.c_str(),root);
    if(flag == false) {
        throw runtime_error("Parse Serialize Json File failed !");
        cerr << "Parse Serialize Json File failed !" << endl;
    }
}

