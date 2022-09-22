//
// Created by 11465 on 2022/6/19.
//

#ifndef SERVER_LOCK_H
#define SERVER_LOCK_H
#include<pthread.h>



namespace server{


class Mutex{
private:
    pthread_mutex_t mMutex;
public:
    Mutex();
    ~Mutex();
    void lock();
    void unlock();
    pthread_mutex_t& getMutex ();
};


class LockGuard{
private:
    Mutex& mMutex;  //用引用，不然传入的mutex不会真的被改变
public:
    explicit LockGuard(Mutex& mutex);
    ~LockGuard();
};


class ConditionVariable{
private:
    pthread_cond_t mCond;
    Mutex& mLock;
public:
    explicit ConditionVariable(Mutex& mutex);
    ~ConditionVariable();
    void nodify();
    void nodifyAll();
    void wait();
};


}




#endif //SERVER_LOCK_H
