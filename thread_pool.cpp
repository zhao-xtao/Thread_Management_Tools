#include <iostream>
#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <unordered_map>
#include <mutex>
#include <condition_variable>

#define BEGINS(x) namespace x {
#define ENDS(x) }

BEGINS(thread_pool)

class Task {
public :
    template<typename FUNCTION, typename ...ARGS>
    Task(FUNCTION &&func, ARGS ...args);
    void run();

private :
    std::function<void()> func;
};

template<typename FUNCTION, typename ...ARGS>
Task::Task(FUNCTION &&func, ARGS ...args) {
    this->func = std::bind(func, std::forward<ARGS>(args)...);
}

void Task::run() {
    std::cout << "Task run : " << std::endl;
    func();
    return ;
}

class ThreadPool {
public :
    ThreadPool(int n = 5);
    template<typename FUNCTION, typename ...ARGS>
    void addOneTask(FUNCTION &&, ARGS...);
    void stop();

private :
    std::vector<std::thread *> threads;
    std::queue<Task *> tasks;
    std::unordered_map<std::thread::id, bool> running;
    std::mutex m_mutex;
    std::condition_variable m_cond;

    void worker_thread();
    Task *getOneTask();
    void stop_thread();
};

ThreadPool::ThreadPool(int n) {
    for (int i = 0; i < n; i++) {
        threads.push_back(new std::thread(&ThreadPool::worker_thread, this));
    }
    return ;
}

void ThreadPool::worker_thread() {
    std::thread::id id = std::this_thread::get_id();
    running[id] = true;
    while(running[id]) {
        //取任务
        Task *t = this->getOneTask();
        //执行任务
        t->run();
        //销毁任务
        delete t;
    }
    return ;
}

template<typename FUNCTION, typename ...ARGS>
void ThreadPool::addOneTask(FUNCTION &&func, ARGS... args) {
    std::unique_lock<std::mutex> lock(m_mutex);
    tasks.push(new Task(func, std::forward<ARGS>(args)...));
    m_cond.notify_one();
    return ;
}

Task *ThreadPool::getOneTask() {
    std::unique_lock<std::mutex> lock(m_mutex);
    while(tasks.empty()) {
        m_cond.wait(lock);
    }
    Task *t = tasks.front();
    tasks.pop();
    return t;
}

void ThreadPool::stop_thread() {
    std::thread::id id = std::this_thread::get_id();
    running[id] = false;
    return ;
}

void ThreadPool::stop() {
    for (int i = 0; i < threads.size(); i++) {
        this->addOneTask(&ThreadPool::stop_thread, this);
    }
    for (auto t : threads) {
        t->join();
    }
    return ;
}

ENDS(thread_pool)

void func(int a, int b, int c) {
    std::cout << "func id :  " << a << ", " << a + b + c << std::endl;
    return ;
}

int main() {
    thread_pool::ThreadPool tp;
    for (int i = 0; i < 100; i++) {
        tp.addOneTask(func, i, 2 * i, 3 * i);
    }
    tp.stop();
    return 0;
}
