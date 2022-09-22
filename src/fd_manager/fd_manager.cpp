#include "fd_manager.h"


namespace server {

FdCtx::FdCtx(int fd) 
: mfd(fd), mRecvTimeOut(-1), mSendTimeOut(-1)
, isClose(false), isSocket(false), isNonBlock(false) 
{
    init();
}

FdCtx::~FdCtx() {
    isClose = true;
}

void FdCtx::init() {
    struct stat fd_stat;
    if(-1 == fstat(mfd, &fd_stat)) {
        isSocket = false;
    } else {
        isSocket = S_ISSOCK(fd_stat.st_mode);
    }

    if(isSocket) {
        int flags = fcntl(mfd, F_GETFL, 0);
        fcntl(mfd, F_SETFL, flags | O_NONBLOCK);
        isNonBlock = true;
    } else {
        isNonBlock = false;
    }
}

void FdCtx::setTimeOut(int type, uint32_t t) {
    if(type) {
        mSendTimeOut = t;
    } else {
        mRecvTimeOut = t;
    }
}

int FdCtx::getTimeOut(int type) {
    return type ? mSendTimeOut : mRecvTimeOut;
}

FdManager::ptr FdManager::fdManager(new FdManager);


void FdManager::add(int fd) {
    mLock.lock();
    if(mCtxs.find(fd) == mCtxs.end()) {
        mCtxs[fd] = FdCtx::ptr(new FdCtx(fd));
    }
    mLock.unlock();
}

void FdManager::del(int fd) {
    mLock.lock();
    if(mCtxs.find(fd) != mCtxs.end()) {
        mCtxs.erase(fd);
    }
    mLock.unlock();
}

FdCtx::ptr FdManager::get(int fd) {
    mLock.lock();
    if(mCtxs.find(fd) == mCtxs.end()) {
        mLock.unlock();
        return nullptr;
    } else {
        mLock.unlock();
        return mCtxs[fd];
    }
}

FdManager::ptr& FdManager::getInstance() {
    return fdManager;
}


}