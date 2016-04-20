#include "ThreadPool.h"

ThreadPool::ThreadPool() : m_pool_size(DEFAULT_POOL_SIZE) {
    m_pOutLog = NULL;
    m_threads.clear();
    m_run_threads.clear();
}

ThreadPool::ThreadPool(int pool_size) : m_pool_size(pool_size) {
    m_pOutLog = NULL;
    m_threads.clear();
    m_run_threads.clear();
}

ThreadPool::~ThreadPool() {
    if (m_pool_state != STOPPED) {
        cout << "~ ThreadPool and Still Running !" << endl;
        destroy_threadpool();
    }
}

void ThreadPool::setPoolSize(const int pool_size) {
    m_pool_size = pool_size;
}

void ThreadPool::setLogOfstream(ofstream &out) {
    m_pOutLog = &out;
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
            *m_pOutLog << "pthrad_create() failed " << ret << endl;
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
    *m_pOutLog << "\nBroadcasting STOP signal to all threads..." << endl;
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
            //*m_pOutLog << "Wait for the condition mutex unlock !" << endl;
            m_task_cond_var.wait(m_task_mutex.get_mutex_ptr());
        }

        // If the thread was woken up to notify process shutdown, return from here
        if(m_pool_state == STOPPED) {
            m_task_mutex.unlock();
            pthread_exit(NULL);
        }

        *m_pOutLog << "Residue the task numebr " << m_tasks.size() << endl;

        task = m_tasks.front();
        m_tasks.pop_front();
        if(m_run_threads.count(thread_id) == 0) {
            m_run_threads.insert(thread_id);
        } else {
            *m_pOutLog << "Thread id " << thread_id << " has already run !" << endl;
        }
        m_task_mutex.unlock();

        task->run();
        m_run_threads.erase(thread_id);
    }
    return NULL;
}

int ThreadPool::add_task(Task* task, const string &task_id) {
    m_task_mutex.lock();
    m_tasks.push_back(task);
    *m_pOutLog << "Now The Task Size is " << m_tasks.size() << endl;
    taskMap[task_id] = task;
    // wake up one thread that is waiting for a task to be available
    m_task_cond_var.signal();
    m_task_mutex.unlock();
    return 0;
}

bool ThreadPool::is_aready_In_Map(const string &task_id) {
    if(taskMap.count(task_id) == 0)
        return false;
    return true;
}

