//
// Created by 11465 on 2022/6/19.
//

#include "lock.h"


namespace server{


server::Mutex::Mutex() {
    pthread_mutex_init(&mMutex, nullptr);
}

server::Mutex::~Mutex() {
    pthread_mutex_destroy(&mMutex);
}

void server::Mutex::lock() {
    pthread_mutex_lock(&mMutex);
}

void server::Mutex::unlock() {
    pthread_mutex_unlock(&mMutex);
}

pthread_mutex_t& server::Mutex::getMutex() {
    return mMutex;
}



LockGuard::LockGuard(Mutex& mutex) : mMutex(mutex) {
    mMutex.lock();
}

LockGuard::~LockGuard() {
    mMutex.unlock();
}



ConditionVariable::ConditionVariable(Mutex& mutex) : mLock(mutex){
    pthread_cond_init(&mCond, nullptr);
}

ConditionVariable::~ConditionVariable() {
    pthread_cond_destroy(&mCond);
}

void ConditionVariable::nodify() {
    pthread_cond_signal(&mCond);
}

void ConditionVariable::wait() {
    pthread_cond_wait(&mCond, &mLock.getMutex());
}

void ConditionVariable::nodifyAll() {
    pthread_cond_broadcast(&mCond);
}



}

