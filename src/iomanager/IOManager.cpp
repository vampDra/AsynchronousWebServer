#include "IOManager.h"

namespace server {

Logger::ptr& IOMLogger = GET_LOG_INSTANCE;

IOManager::Event::EventCtx &IOManager::Event::getEvent(uint32_t ev) {
    switch (ev) {
        case EPOLLIN:
            return read;
        case EPOLLOUT:
            return write;
        default:
            LOG_ERROR(IOMLogger) << "event fault";
            exit(1);
    }
}

void IOManager::Event::triggerEvent(uint32_t ev) {
    event = event & ~ev;        //去除当次任务
    EventCtx& ctx = getEvent(ev);
    if (ctx.fiber) {
        IOManager::getCurIOManager()->addTask(ctx.fiber);
    } else if (ctx.cb) {
        IOManager::getCurIOManager()->addTask(ctx.cb);
    }
    reset(ev);
}

void IOManager::Event::reset(uint32_t ev) {
    EventCtx &task = getEvent(ev);
    task.fiber.reset();
    task.cb = nullptr;
}

IOManager::IOManager(int threadCnt, bool mainThrd, int slotnum, int ticktime)
: Scheduler(threadCnt)
, mUseMainThrd(mainThrd)
, mTickTime(ticktime)
, mSlotCnt(slotnum) {
    mTimerManager = new TimerManager(slotnum, ticktime);
    mEpollfd = epoll_create(5);
    mEventfd = eventfd(0, EFD_NONBLOCK);

    epoll_event event;
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = mEventfd;

    epoll_ctl(mEpollfd, EPOLL_CTL_ADD, mEventfd, &event);
    ctxResize(128);

    start();
}

IOManager::~IOManager() {
    if (mUseMainThrd && !stopping()) {
        threadFunc();
    }
    join();
    for(Event* item : mEvents) {
        if(item) {
            delete item;
            item = nullptr;
        }
    }
    delete mTimerManager;
    close(mEpollfd);
    close(mEventfd);
}

bool IOManager::stopping() {
    return !mTimerCnt && Scheduler::stopping();
}

void IOManager::ctxResize(int size) {
    mEvents.resize(size);
    for(int i = 0; i < mEvents.size(); i++) {
        if(!mEvents[i]) {
            mEvents[i] = new Event;
            mEvents[i]->fd = i, mEvents[i]->event = 0;
        }
    }
    mEventCnt = mEvents.size();
}

void IOManager::addEvent(int fd, uint32_t event, std::function<void()> cb, int flag) {
    mLock.lock();
    if(fd >= (int)mEventCnt * 0.8) {
        ctxResize((int)(fd * 1.5));
    }
    mLock.unlock();
    Event* task = mEvents[fd];
    if(task->event & event) {
        return;
    }

    //挂上事件监听
    int op = task->event == 0 ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;
    epoll_event ev;
    ev.events = flag | event | task->event;
    ev.data.ptr = task;
    int rt = epoll_ctl(mEpollfd, op, fd, &ev);
    if(rt) {
        LOG_ERROR(IOMLogger) << "epoll_ctl error,errno=" << errno << " " 
                             << strerror(errno) << ", fd=" << fd << ",op=" << op;
        return;
    }

    task->lock.lock();
    task->event |= event;
    Event::EventCtx &ctx = task->getEvent(event);        //获取相应事件
    if(cb) {
        ctx.cb = cb;
    } else {
        ctx.fiber = Fiber::getCurFiber();     
    }
    task->lock.unlock();
}

void IOManager::cancelEvent(int fd, uint32_t event) {
    if(fd >= mEventCnt) {
        return;
    }
    Event *task = mEvents[fd];
    task->lock.lock();
    if(task->event & event) {
        int op = ((task->event  & ~event) == 0) ? EPOLL_CTL_DEL : EPOLL_CTL_MOD;
        epoll_event epev;
        epev.events = task->event | EPOLLET;
        epev.data.ptr = task;
        int rt = epoll_ctl(mEpollfd, op, fd, &epev);
        if(rt) {
            LOG_ERROR(IOMLogger) << "epoll_ctl error,errno=" << errno << " " 
                                 << strerror(errno) << ", fd=" << fd << ",op=" << op;
        }
        task->triggerEvent(event);
    }
    task->lock.unlock();
}

void IOManager::delEvent(int fd) {
    if(fd >= mEventCnt) {
        return;
    }
    Event *task = mEvents[fd];
    //无事件证明已被取消过
    if(!task->event) {
        return;
    }
    int rt = epoll_ctl(mEpollfd, EPOLL_CTL_DEL, fd, nullptr);
    if(rt) {
        LOG_ERROR(IOMLogger) << "epoll_ctl error,errno=" << errno << " " 
                             << strerror(errno) << ", fd=" << fd;
    }
    task->lock.lock();
    task->event = 0x000;
    task->reset(EPOLLIN);
    task->reset(EPOLLOUT);
    task->lock.unlock();
}

Timer::ptr IOManager::addTimer(int timing, std::function<void()> cb, bool recurr) {
    mTimerCnt++;
    return mTimerManager->addTimer(timing, cb, recurr);
}

void IOManager::delTimer(Timer::ptr timer) {
    mTimerCnt--;
    mTimerManager->delTimer(timer);
}

void IOManager::trickle() {
    uint64_t buf = 1;
    int rt = write(mEventfd, &buf, sizeof(buf));
    if(rt != sizeof(buf)) {
        LOG_ERROR(IOMLogger) << "eventfd write error\n"
                             << "eventfd rt=" << rt
                             << " , errno=" << errno << ", "
                             << strerror(errno);
    }
}

void IOManager::idle() {
    const int waitCnt = 2048;
    epoll_event *eps = new epoll_event[waitCnt];
    std::shared_ptr<epoll_event> shared_events(eps, [](epoll_event *ptr) {
        delete[] ptr;
    });

    int waitTime = mTickTime;       //epoll_wait时间间隔
    while(true) {
        bool timeout = false;       //定时器到期
        uint64_t begin, end;        //epoll_wait起始结束时间

        begin = getCurTimeMS();
        int num = epoll_wait(mEpollfd, eps, waitCnt, waitTime);
        if(num < 0 && errno == EINTR) {
            continue;
        }
        //定时器到时返回
        if(num == 0) {
            timeout = true;
        }
        //处理读写事件
        else if(num > 0) {
            //遍历触发事件
            for(int i = 0; i < num; i++) {
                epoll_event &event = eps[i];
                //触发eventfd事件通知
                if(event.data.fd == mEventfd) {
                    uint64_t buf;
                    while(read(mEventfd, &buf, sizeof buf) != -1);
                    LOG_DEBUG(IOMLogger) << "handle eventfd";
                    continue;
                }
                //处理读写事件
                Event *task = (Event*)event.data.ptr;
                LockGuard lock(task->lock);
                uint32_t realEvent = 0x000;
                uint32_t leftEvent;
                if(event.events & EPOLLIN) {
                    realEvent |= EPOLLIN;
                }
                if(event.events & EPOLLOUT) {
                    realEvent |= EPOLLOUT;
                }
                if((task->event & realEvent) == 0x000) {
                    continue;
                }
                leftEvent = task->event & ~realEvent;     //获取剩余事件
                event.events = EPOLLET | leftEvent;
                int op = leftEvent == 0 ? EPOLL_CTL_DEL : EPOLL_CTL_MOD;
                int rt = epoll_ctl(mEpollfd, op, task->fd, &event);
                if(rt) {
                    LOG_ERROR(IOMLogger) << "epoll_ctl error,errno=" << errno << " " 
                                         << strerror(errno) << ", fd=" << task->fd << ",op=" << op;
                    continue;
                }

                if(realEvent & EPOLLIN) {
                    task->triggerEvent(EPOLLIN);
                }
                if(realEvent & EPOLLOUT) {
                    task->triggerEvent(EPOLLOUT);
                }
            }
            //判断是否超时
            end = getCurTimeMS();
            waitTime -= (end - begin);
            //处理定时任务
            if(waitTime <= 0) {
                timeout = true;
            }
        }
        //错误处理
        else {
            LOG_ERROR(IOMLogger) << "epoll_wait error, errno=" << errno
                                 << " ," << strerror(errno);
            break;
        }
        //处理定时器任务
        if(timeout) {
            std::vector<std::function<void()>> cb;
            mTimerManager->tick(cb);
            addTask(cb.begin(), cb.end());
            waitTime = mTickTime;
        }
        Fiber::getCurFiber()->yield();
    }
}

}