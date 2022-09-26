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

FdManager::FdManager() {
    mCtxs.resize(64);
}

// void FdManager::add(int fd) {
//     mLock.lock();
//     if(mCtxs.find(fd) == mCtxs.end()) {
//         mCtxs[fd] = FdCtx::ptr(new FdCtx(fd));
//     }
//     mLock.unlock();
// }

void FdManager::del(int fd) {
    LockGuard lock(mLock);
    if((int)mCtxs.size() <= fd) {
        return;
    }
    mCtxs[fd].reset();
}

FdCtx::ptr FdManager::get(int fd, bool auto_create) {
    if(fd == -1) {
        return nullptr;
    }
    LockGuard lock(mLock);
    if((int)mCtxs.size() <= fd) {
        if(auto_create == false) {
            return nullptr;
        }
    } else {
        if(mCtxs[fd] || !auto_create) {
            return mCtxs[fd];
        }
    }
    FdCtx::ptr ctx(new FdCtx(fd));
    if(fd >= (int)mCtxs.size()) {
        mCtxs.resize(fd * 1.5);
    }
    mCtxs[fd] = ctx;
    return ctx;
}

FdManager::ptr& FdManager::getInstance() {
    return fdManager;
}

}