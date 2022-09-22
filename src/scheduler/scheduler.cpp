#include "scheduler.h"


namespace server {

Logger::ptr& scheduleLogger = GET_LOG_INSTANCE;
Scheduler* curScheduler = nullptr;

Scheduler::Scheduler(int threadCnt) : mThreadCnt(threadCnt) {
    if(mThreadCnt < 1) mThreadCnt = 1;
}

Scheduler::~Scheduler() {
    if(curScheduler == this) {
        curScheduler = nullptr;
    }
}

Scheduler* Scheduler::getCurScheduler() {
    return curScheduler;
}

void Scheduler::start() {
    mLock.lock();
    curScheduler = this;
    for(int i = 0; i < mThreadCnt; i++) {
        mThreads.emplace_back(std::thread(std::bind(&Scheduler::threadFunc, this)));
        mThreadIds.push_back(mThreads[i].get_id());
        LOG_DEBUG(scheduleLogger) << "create thread: " << mThreadIds[i];
    }
    mLock.unlock();
}

void Scheduler::join() {
    for (int i = 0; i < mThreadCnt; i++) {
        mThreads[i].join();
        LOG_DEBUG(scheduleLogger) << "tid: " << mThreadIds[i] << " join";
    }
}

void Scheduler::trickle() {
    LOG_DEBUG(scheduleLogger) << "trickle";
}

void Scheduler::idle() {
    while(1) {
        Fiber::yieldToHold();
    }
}

bool Scheduler::stopping() {
    //结束标记 && 没有任务 && 线程空闲 && 没有定时器
    return mStopping && mTasks.empty() && !mActiveCnt && !mTimerCnt;
}

void Scheduler::stop() {
    mStopping = true;
}

void Scheduler::threadFunc() {
    Task::ptr task(new Task());       //暂存当前执行任务
    Fiber::ptr mainf = Fiber::getCurFiber();        //创建线程主协程
    Fiber::ptr idleFiber(new Fiber(std::bind(&Scheduler::idle, this), 1024 * 1024));    //空闲协程
    Fiber::ptr cbfiber = nullptr;
    while(true) {
        task->reset();
        //选取相应的任务进行执行
        bool isTrickle = false;
        {
            mLock.lock();
            for(auto it = mTasks.begin(); it != mTasks.end(); it++) {
                //不属于当前线程执行
                if((*it)->threadNum != -1 && std::this_thread::get_id() != mThreadIds[(*it)->threadNum]) {
                    isTrickle |= true;
                    continue;
                }
                task = *it;
                mTasks.erase(it);
                ++mActiveCnt;
                break;
           }
            mLock.unlock();
            if(!mTasks.empty()) isTrickle |= true;
        }
        if(isTrickle) trickle();
        // 执行体为协程，这里要约定好，如果是中途yield，是协程函数自己添加还是调度器帮忙添加，
        // 我这里是调度器帮忙添加
        if(task->fiber) {
            task->fiber->resume();
            if(task->fiber->getState() == Fiber::HOLD) {
                addTask(task->fiber, task->threadNum);
            }
            --mActiveCnt;
        }
        //执行体为函数，构建新协程处理
        else if(task->func) {
            if(!cbfiber) {
                cbfiber.reset(new Fiber(task->func));
            } else {
                cbfiber->reset(task->func);
            }
            cbfiber->resume();
            if(cbfiber->getState() == Fiber::HOLD) {
                addTask(cbfiber, task->threadNum);
            }
            cbfiber.reset();
            --mActiveCnt;
        }
        //无执行内容
        else {
            idleFiber->resume();    //切换入epoll
            if(stopping()) {
                LOG_DEBUG(scheduleLogger) << "tid: " << std::this_thread::get_id() << " stop";
                break;
            }
        }
    }
}


}
