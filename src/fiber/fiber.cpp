#include "fiber.h"
#include "scheduler.h"

namespace server{

Logger::ptr& g_logger = GET_LOG_INSTANCE;
static std::atomic<uint64_t> s_fiber_id{0};
static std::atomic<uint64_t> s_fiber_count{0};
static thread_local Fiber *t_fiber = nullptr;
static thread_local Fiber::ptr t_thread_fiber = nullptr;

uint64_t Fiber::GetFiberId() {
    if (t_fiber) {
        return t_fiber->getId();
    }
    return 0;
}

Fiber::Fiber() {
    SetThis(this);
    m_state = EXEC;

    if (getcontext(&m_ctx)) {
    }

    ++s_fiber_count;
    m_id = s_fiber_id++; 

    LOG_DEBUG(g_logger) << "Fiber::Fiber() main id = " << m_id;
}

void Fiber::SetThis(Fiber *f) { 
    t_fiber = f; 
}

Fiber::ptr Fiber::getCurFiber() {
    if (t_fiber) {
        return t_fiber->shared_from_this();
    }

    Fiber::ptr main_fiber(new Fiber);
    t_thread_fiber = main_fiber;
    t_fiber = main_fiber.get();
    return t_fiber->shared_from_this();
}

Fiber::Fiber(std::function<void()> cb, size_t stacksize)
: m_id(s_fiber_id++)
, m_cb(cb) {
    ++s_fiber_count;
    m_stacksize = stacksize;
    m_stack  = malloc(stacksize);

    getcontext(&m_ctx);

    m_ctx.uc_link = nullptr;
    m_ctx.uc_stack.ss_sp   = m_stack;
    m_ctx.uc_stack.ss_size = m_stacksize;
    makecontext(&m_ctx, &Fiber::MainFunc, 0);

    LOG_DEBUG(g_logger) << "Fiber::Fiber() id = " << m_id;
}

Fiber::~Fiber() {
    LOG_DEBUG(g_logger) << "Fiber::~Fiber() id = " << m_id;
    --s_fiber_count;
    if (m_stack) {
        free(m_stack);
        LOG_DEBUG(g_logger) << "dealloc stack, id = " << m_id;
    } else {
        Fiber *cur = t_fiber; // 当前协程就是自己
        if (cur == this) {
            SetThis(nullptr);
        }
    }
}

void Fiber::reset(std::function<void()> cb) {
    m_cb = cb;
    getcontext(&m_ctx);
    m_ctx.uc_link = nullptr;
    m_ctx.uc_stack.ss_sp   = m_stack;
    m_ctx.uc_stack.ss_size = m_stacksize;

    makecontext(&m_ctx, &Fiber::MainFunc, 0);
    m_state = HOLD;
}

void Fiber::resume() {
    SetThis(this);
    m_state = EXEC;

    if (swapcontext(&(Scheduler::getMainFiber()->m_ctx), &m_ctx)) {
        LOG_ERROR(g_logger) << "swap error";
    }
}

void Fiber::yield() {
    SetThis(t_thread_fiber.get());
    if (m_state != TERM) {
        m_state = HOLD;
    }

    if (swapcontext(&m_ctx, &(Scheduler::getMainFiber()->m_ctx))) {
        LOG_ERROR(g_logger) << "swap error";
    }
}

void Fiber::yieldToHold() {
    getCurFiber()->yield();
}

void Fiber::MainFunc() {
    Fiber::ptr cur = getCurFiber(); 

    cur->m_cb();
    cur->m_cb = nullptr;
    cur->m_state = TERM;

    auto raw_ptr = cur.get();
    cur.reset();
    raw_ptr->yield();
}

}
