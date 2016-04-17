#include "ThreadPool.h"

ThreadPool::ThreadPool() : m_pool_size(DEFAULT_POOL_SIZE) {
    ;
}

ThreadPool::ThreadPool(int pool_size) : m_pool_size(pool_size) {
    ;
}

ThreadPool::~ThreadPool() {
    if (m_pool_state != STOPPED) {
        destroy_threadpool();
    }
}

void ThreadPool::setPoolSize(const int pool_size) {
    m_pool_size = pool_size;
}

// We can't pass a member function to pthread_create.
// So created the wrapper function that calls the member function
// we want to run in the thread.
extern "C"
void* start_thread(void* arg) {
    ThreadPool* tp = (ThreadPool*) arg;
    tp->execute_thread();
    return NULL;
}

int ThreadPool::initialize_threadpool() {
    m_pool_state = STARTED;
    int ret = -1;
    for(int i = 0; i < m_pool_size; i++) {
        pthread_t tid;
        ret = pthread_create(&tid, NULL, start_thread, (void*) this);
        if (ret != 0) {
            cerr << "pthread_create() failed: " << ret << endl;
            return -1;
        }
        m_threads.push_back(tid);
    }
    cout << m_pool_size << " threads created by the thread pool" << endl;

    return 0;
}

int ThreadPool::destroy_threadpool() {
    m_task_mutex.lock();
    m_pool_state = STOPPED;
    m_task_mutex.unlock();
    cout << "\nBroadcasting STOP signal to all threads..." << endl;
    m_task_cond_var.broadcast(); // notify all threads we are shttung down

    int ret = -1;
    for (int i = 0; i < m_pool_size; i++) {
        void* result;
        ret = pthread_join(m_threads[i], &result);
        m_task_cond_var.broadcast(); // try waking up a bunch of threads that are still waiting
    }
    cout << m_pool_size << " threads exited from the thread pool" << endl;
    return 0;
}

int ThreadPool::runningNumbers() {
    return m_tasks.size();
}

int ThreadPool::getPoolCapacity() {
    return m_pool_size;
}

void* ThreadPool::execute_thread() {
    Task* task = NULL;
    while(true) {
        // Try to pick a task
        m_task_mutex.lock();
        cout << "Locking: " << pthread_self() << endl;

        while((m_pool_state != STOPPED) && (m_tasks.empty())) {
            m_task_cond_var.wait(m_task_mutex.get_mutex_ptr());
        }

        // If the thread was woken up to notify process shutdown, return from here
        if(m_pool_state == STOPPED) {
            m_task_mutex.unlock();
            pthread_exit(NULL);
        }

        task = m_tasks.front();
        m_tasks.pop_front();
        cout << "Unlocking: " << pthread_self() << endl;
        m_task_mutex.unlock();

        task->run();
        delete task;
    }
    return NULL;
}

int ThreadPool::add_task(Task* task) {
    m_task_mutex.lock();
    m_tasks.push_back(task);
    m_task_cond_var.signal(); // wake up one thread that is waiting for a task to be available
    m_task_mutex.unlock();
    return 0;
}
