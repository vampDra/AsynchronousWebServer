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
        HOLD,
        EXEC,
        TERM
    };
private:
    /**
     * @brief 构造函数
     * @attention 无参构造函数只用于创建线程的第一个协程，也就是线程主函数对应的协程，
     * 这个协程只能由GetThis()方法调用，所以定义成私有方法
     */
    Fiber();
public:
    /**
     * @brief 构造函数，用于创建用户协程
     * @param[in] cb 协程入口函数
     * @param[in] stacksize 栈大小
     * @param[in] run_in_scheduler 本协程是否参与调度器调度，默认为true
     */
    Fiber(std::function<void()> cb, size_t stacksize = 1024 * 1024);
    ~Fiber();
    /**
     * @brief 重置协程状态和入口函数，复用栈空间，不重新创建栈
     * 
     */
    void reset(std::function<void()> cb);
    void resume();
    void yield();
    uint64_t getId() const { return m_id; }
    State getState() const { return m_state; }

public:
    static void SetThis(Fiber *f);
    static Fiber::ptr getCurFiber();
    static uint64_t TotalFibers();
    static void yieldToHold();
    static void MainFunc();
    static uint64_t GetFiberId();
private:
    std::function<void()> m_cb;
    uint32_t m_stacksize = 0;
    uint64_t m_id = 0;
    ucontext_t m_ctx;
    State m_state = HOLD;
    void *m_stack = nullptr;
};

}

#endif //SERVER_FIBER_H