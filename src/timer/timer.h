#ifndef __TIMER_H__
#define __TIMER_H__
#include "lock.h"
#include "logger.h"
#include <memory>
#include <functional>
#include <sys/time.h>
#include <vector>
#include <list>


namespace server {
//获取当前时间 （毫秒）
uint64_t getCurTimeMS();

struct Timer {
    typedef std::shared_ptr<Timer> ptr;
    int intervalTime;                   //定时时间
    int round;                          //第几轮执行
    int slot;                           //存储在第几个槽
    std::function<void()> callback;     //回调函数
    bool recurring;                     //是否循环执行

    Timer(int interval, int round, int slot, std::function<void()> callback, bool recurr = false)
    : intervalTime(interval), round(round), slot(slot), callback(callback), recurring(recurr) {}
};

//时间轮的方式做定时器容器，实现方式像散列加上邻接表
class TimerManager {
public:
    TimerManager(int slotnum, int ticktime);
    ~TimerManager();
    void tick(std::vector<std::function<void()>> &cbs);        //滴答函数
    Timer::ptr addTimer(int timing, std::function<void()> cb, bool recurr = false);
    void delTimer(Timer::ptr timer); 
private:
    Mutex mLock;
    int mSlotNum;                             //总槽数
    int mTickTime;                            //滴答时间,经过一次滴答时间，槽指针向前移动一格
    int mCurSlot;                             //当前槽指针位置
    uint64_t mLastExpireTime;                 //上一次tick时间
    std::vector<std::list<Timer::ptr>> mWheel;    //时间轮容器
};


}


#endif //SERVER_TIMER_H