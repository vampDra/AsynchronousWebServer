#include "fiber.h"


namespace server{

Logger::ptr& fiberLogger = GET_LOG_INSTANCE;
thread_local Fiber::ptr curFiber  = nullptr;      //当前正在执行的协程
thread_local Fiber::ptr mainFiber = nullptr;      //主协程
std::atomic<int> Fiber::mFiberId    {0};
std::atomic<int> Fiber::mFiberCount {0};


Fiber::Fiber() {
    mState = EXEC;
    mFiberCount++;
    mID = 0;
    getcontext(&mCtx);

    LOG_DEBUG(fiberLogger) << "tid: " << std::this_thread::get_id() << ", create main fiber" ;
}

//构建子协程
Fiber::Fiber(std::function<void()> cb, uint64_t stackSize, int thread) 
: mCallBack(cb)
, mStackSize(stackSize)
, mID(++mFiberId)
, mState(HOLD)
, mThread(thread) {
    mFiberCount++;
    mStack = malloc(mStackSize);

    mCtx.uc_stack.ss_sp   = mStack;
    mCtx.uc_stack.ss_size = mStackSize;
    if(mainFiber) {
        mCtx.uc_link = &mainFiber->mCtx;      //当前创建了线程主协程的话，再指向他
    }
    getcontext(&mCtx);
    makecontext(&mCtx, func, 0);

    LOG_DEBUG(fiberLogger) << "tid: " << std::this_thread::get_id() << " , create fiber id: " << mID;
}

//分主协程与子协程析构
Fiber::~Fiber() {
    mFiberCount--;
    //析构子协程
    if(mStack) {
        free(mStack);
        LOG_DEBUG(fiberLogger) << std::this_thread::get_id() << " , ~Fiber id: " << mID;
    }
    else {
        setCurFiber(nullptr);
        LOG_DEBUG(fiberLogger)  << std::this_thread::get_id() << " , ~Fiber main";
    }
}

//重置回调函数
void Fiber::reset(std::function<void()> cb, int thread) {
    mState = HOLD;
    mCallBack = cb;
    mCtx.uc_stack.ss_sp   = mStack;
    mCtx.uc_stack.ss_size = mStackSize;
    mCtx.uc_link = &mainFiber->mCtx;
    setThread(thread);
    
    makecontext(&mCtx, func, 0);
    LOG_DEBUG(fiberLogger)  << std::this_thread::get_id() << " , reset fiber id: " << mID;
}

void Fiber::setCurFiber(Fiber::ptr cur) {
    curFiber = cur;
}

Fiber::ptr Fiber::getCurFiber() {
    if(curFiber) {
        return curFiber;
    }
    mainFiber.reset(new Fiber);
    curFiber = mainFiber;
    return curFiber;
}

void Fiber::resume() {
    setCurFiber(this->shared_from_this());
    mState = EXEC;
    LOG_DEBUG(fiberLogger)  << std::this_thread::get_id() << " main, swap to id: " << mID;

    if(swapcontext(&mainFiber->mCtx, &mCtx) == -1) {
        LOG_ERROR(fiberLogger) << std::this_thread::get_id() << " , swap error, id: " << mID;
    }
}

void Fiber::yield() {
    setCurFiber(mainFiber);
    LOG_DEBUG(fiberLogger)  << std::this_thread::get_id() << " , id: " << mID << " swap to main";

    if(swapcontext(&mCtx, &mainFiber->mCtx) == -1) {
        LOG_ERROR(fiberLogger) << std::this_thread::get_id() << " , swap error, id: " << mID << " to main";
    }
}

void Fiber::yieldToHold() {
    if(curFiber->mState == EXEC){
        curFiber->mState = HOLD;
        curFiber->yield();
    }
    else LOG_ERROR(fiberLogger)  << std::this_thread::get_id() << " , id: " << curFiber->mID << "is not exec";
}

void Fiber::func() {
    curFiber->mCallBack();
    curFiber->mCallBack = nullptr;
    curFiber->mState = TERM;
    curFiber->yield();         
}

int Fiber::gerFiberCnt() {
    return mFiberCount;
}

}