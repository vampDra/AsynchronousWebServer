#ifndef __FIBER_H__
#define __FIBER_H__
#include "logger.h"
#include <ucontext.h>
#include <memory>
#include <cstring>
#include <atomic>
#include <functional>


namespace server{
class Fiber : public std::enable_shared_from_this<Fiber> {
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
    Fiber(std::function<void()>cb, uint64_t stackSize = 1024 * 1024);     //构建子协程
    ~Fiber();
    void reset (std::function<void()>cb);     //重置协程执行函数
    void resume();                            //换到当前协程执行
    void yield ();                            //切换回主协程
    int  getId() {return mId;}                
    State getState() {return mState;}
public:
    static Fiber::ptr getCurFiber();          //获取当前正在执行的协程
    static int gerFiberCnt ();                //获取总协程数
    static void yieldToHold();                //切换回主协程
    static void func();                       //协程执行函数
private:
    std::function<void()> mCallBack;          //回调函数
    uint32_t mId;                             //协程id
    uint64_t mStackSize;                      //栈大小
    ucontext_t mCtx;                          //协程上下文
    void* mStack = nullptr;                   //执行栈
    State mState = HOLD;
private:
    static std::atomic<int> mFiberId;
    static std::atomic<int> mFiberCount;
};

}

#endif //SERVER_FIBER_H