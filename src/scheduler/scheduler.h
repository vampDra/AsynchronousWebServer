#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__
#include "fiber.h"
#include <memory>
#include <functional>
#include <iostream>
#include <vector>
#include <thread>
#include <unistd.h>
#include <list>


namespace server {
/**
 * @brief 协程调度器，N线程调度M个协程
 * 本类仅做协程调度，不涉及具体模型处理
 */
class Scheduler {
public:
    typedef std::shared_ptr<Scheduler> ptr;
    Scheduler(int threadCnt);
    virtual ~Scheduler();
    void start();                    //启动调度器，创建线程
    void stop ();
    static Scheduler* getCurScheduler();

    template<class task>
    void addTask(task t, int threadNum = -1) {
        bool isNull;
        {
            mLock.lock();
            isNull = addTaskNoLock(t, threadNum);
            mLock.unlock();
        }
        if(isNull) trickle();       //唤醒工作线程
    }
    template<class iterator>
    void addTask(iterator begin, iterator end) {
        bool isNull;
        {
            LockGuard lock(mLock);
            //取出迭代器内的元素，智能指针是否删除应该外部来管理，not内部
            while(begin != end){
                isNull |= addTaskNoLock(*begin);
                ++begin;
            }
        }
        if(isNull) trickle();
    }
protected:
    std::atomic<int> mTimerCnt {0};     //定时器数量
    virtual bool stopping();     //是否正在结束
    virtual void trickle ();     //通知函数
    virtual void idle();         //空闲等待函数
    void threadFunc();           //线程入口函数
    void join();
private:
    /*
     * 每个协程既可以随机选择线程，也可以指定线程。
     * 默认-1为随机选择，填数字指定第几个线程执行。
     */
    template<class task>
    bool addTaskNoLock(task t, int threadNum = -1) {
        bool isNull = mTasks.empty();
        if(threadNum > (int)mThreads.size()) {
            threadNum = -1;
        }
        Task::ptr f(new Task(t, threadNum));
        if(f->func || f->fiber) {
            mTasks.emplace_back(f);
        }
        return isNull;
    }
private:
    struct Task {
        typedef std::shared_ptr<Task> ptr;
        Task(Fiber::ptr f, int num)
        : fiber(f), threadNum(num) {}
        Task(std::function<void()> f, int num)
        : func(f), threadNum(num) {}
        Task() = default;
        void reset() {
            fiber.reset(), 
            func  = nullptr;
            threadNum = -1;
        }
        Fiber::ptr fiber = nullptr;
        std::function<void()> func = nullptr;
        int threadNum = -1;     //执行线程, -1为随机
    };
private:
    Mutex mLock;
    std::vector<std::thread::id> mThreadIds;    //线程号与线程一一对应
    std::vector<std::thread> mThreads;          //线程池
    std::list<Task::ptr>  mTasks;               //任务队列
private:
    int mThreadCnt = 0;                         //总线程数
    bool mStopping = false;                     //结束标志
    std::atomic<int> mActiveCnt{0};             //活跃线程数
};


}


#endif //SERVER_SCHEDULE_H