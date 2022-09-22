//
// Created by 11465 on 2022/7/24.
//

#include "timer.h"

namespace server {

Logger::ptr& logger = GET_LOG_INSTANCE;

extern uint64_t getCurTimeMS() {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;    //秒*1000 + 微秒/1000 = 毫秒
}


TimerManager::TimerManager(int slotnum, int ticktime)
: mSlotNum(slotnum), mTickTime(ticktime) {
    mCurSlot = 0;
    mWheel.resize(mSlotNum);
    mLastExpireTime = getCurTimeMS();
}

TimerManager::~TimerManager() {
    for(int i = 0; i < mSlotNum; i ++) {
        auto it = mWheel[i].begin();
        while(it != mWheel[i].end()) {
            it = mWheel[i].erase(it);
        }
    }
}

Timer::ptr TimerManager::addTimer(int timing, std::function<void()>cb, bool recurr) {
    int ticks = timing / mTickTime;                             //滴答数
    int round = ticks / mSlotNum;                               //轮数
    int slot = (mCurSlot + (ticks % mSlotNum)) % mSlotNum;      //存储位置
    Timer::ptr newTimer(new Timer(timing, round, slot, cb, recurr));
    mLock.lock();
    mWheel[slot].emplace_back(newTimer);
    mLock.unlock();
    return newTimer;
}

void TimerManager::delTimer(Timer::ptr timer) {
    mLock.lock();
    mWheel[timer->slot].remove(timer);
    mLock.unlock();
}

void TimerManager::tick(std::vector<std::function<void()>> &cbs) {
    mLock.lock();
    if( (int)(getCurTimeMS() - mLastExpireTime) >= mTickTime ) {
        auto it = mWheel[mCurSlot].begin();
        std::vector<Timer::ptr> recurTimer;

        while(it != mWheel[mCurSlot].end()) {
            auto tmp = *it;
            if(tmp->round) {
                tmp->round--;
                continue;
            }
            cbs.emplace_back(tmp->callback);
            if(tmp->recurring) {
                recurTimer.emplace_back(tmp);
            }
            it = mWheel[mCurSlot].erase(it);
        }
        //添加循环定时器，感觉这样效率不高
        for(auto item = recurTimer.begin(); item != recurTimer.end(); item++) {
            Timer::ptr tmp = *item;
            //直接调用addTimer()会死锁
            int ticks = tmp->intervalTime / mTickTime;                  //滴答数
            int round = ticks / mSlotNum;                               //轮数
            int slot = (mCurSlot + (ticks % mSlotNum)) % mSlotNum;      //存储位置
            tmp->round = round;
            mWheel[slot].emplace_back(tmp);
        }

        mCurSlot = (mCurSlot + 1) % mSlotNum;
        mLastExpireTime = getCurTimeMS();
    }
    mLock.unlock();
}


}