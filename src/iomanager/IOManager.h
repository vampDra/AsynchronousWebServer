#ifndef __IOMANAGER_H__
#define __IOMANAGER_H__
#include "timer.h"
#include "scheduler.h"
#include <sys/eventfd.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>


namespace server {
/*
 *基于Scheduler封装eventLoop，并结合定时器设置定时任务
 *
 */
class IOManager :
    public Scheduler,
    public std::enable_shared_from_this<IOManager> {
private:
    //event.ptr指向的结构体
    struct Event {
        struct EventCtx {
            std::function<void()> cb;
            Fiber::ptr fiber;
        };
        EventCtx& getEvent(uint32_t ev);   //获取事件
        void triggerEvent (uint32_t ev);   //向调度器添加任务
        void reset(uint32_t ev);
        uint32_t event = 0x000;            //默认null
        EventCtx write;                    //写事件
        EventCtx read;                     //读事件
        Mutex    lock;
        int      fd = 0;                  
    };
public:
    typedef std::shared_ptr<IOManager> ptr;
    IOManager (int threadCnt = 1, bool mainThrd = true,  int slotnum = 60, int ticktime = 200);
    ~IOManager();
    void addEvent(int fd, uint32_t event, 
         std::function<void()> cb = nullptr, int flag = EPOLLET);  //添加监听
    void cancelEvent(int fd, uint32_t event);   //取消描述符上的某一任务
    void delEvent(int fd);                      //取消监听当前文件描述符
    void delTimer(Timer::ptr timer);
    Timer::ptr addTimer(int timing, std::function<void()> cb, bool recurr = false);
    static IOManager *getCurIOManager() {return dynamic_cast<IOManager*>(Scheduler::getCurScheduler());}
private:
    void ctxResize(int size);           //对象池扩容
    bool stopping () override;
    void trickle  () override;          //通知函数
    void idle() override;               //idle协程入口函数(eventLoop)
private:
    std::vector<Event*> mEvents;        //对象池
    TimerManager* mTimerManager;        //定时器管理器
    bool mUseMainThrd;                  //是否使用主线程进行调度
    int mEventCnt = 0;
    int mEpollfd  = 0;                  // epoll接口
    int mEventfd  = 0;                  //跨线程事件通知
    int mSlotCnt;                       //时间轮槽数
    int mTickTime;                      //epoll_wait返回时间 && 定时器tick时间
    Mutex mLock;
};


}


#endif //SERVER_IOMANAGER_H
