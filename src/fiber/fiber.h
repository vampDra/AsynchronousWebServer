#ifndef __FIBER_H__
#define __FIBER_H__
#include "logger.h"
#include <ucontext.h>
#include <memory>
#include <cstring>
#include <atomic>
#include <functional>


namespace server{

class Fiber 
: public std::enable_shared_from_this<Fiber> {
public:
    typedef std::shared_ptr<Fiber> ptr;
    enum State {
        HOLD = 0,            // 暂停状态
        EXEC,                // 执行中状态
        TERM,                // 结束状态
    };
private:
    Fiber();                                  //私有无参构造，使每个线程只有一个主协程
    void setCurFiber(Fiber::ptr cur);         //设置当前线程的执行协程
public:
    Fiber(std::function<void()>cb, uint64_t stackSize = 1024 * 1024, int thread = -1);     //构建子协程
    ~Fiber();
    void reset (std::function<void()>cb, int thread = -1);     //重置协程执行函数
    void resume();                            //换到当前协程执行
    void yield ();                            //切换回主协程
    void setThread(int t) {mThread = t;}
    int  getThread() {return mThread;}                
    State getState() {return mState;}
    int  getID() {return mID;}
public:
    static Fiber::ptr getCurFiber();          //获取当前正在执行的协程
    static int gerFiberCnt ();                //获取总协程数
    static void yieldToHold();                //切换回主协程
    static void func();                       //协程执行函数
private:
    std::function<void()> mCallBack;          //回调函数
    void* mStack = nullptr;                   //执行栈
    uint64_t mStackSize;                      //栈大小
    ucontext_t mCtx;                          //协程上下文
    uint32_t mID;                             //协程id
    State mState;
    int mThread;                              //运行在哪个线程上(子协程) -1为随机
private:
    static std::atomic<int> mFiberId;
    static std::atomic<int> mFiberCount;
};

}

#endif //SERVER_FIBER_H