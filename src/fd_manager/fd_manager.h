#ifndef __FD_MANAGER_H__
#define __FD_MANAGER_H__
#include "lock.h"
#include "hook.h"
#include <arpa/inet.h>
#include <memory>
#include <vector>
#include <sys/stat.h>


namespace server {

class FdCtx {
public:
    typedef std::shared_ptr<FdCtx> ptr;
    FdCtx (int fd);
    ~FdCtx();
    void init();
    bool IsClose () {return isClose;}
    bool IsSocket() {return isSocket;}
    bool IsNonBlock() {return isNonBlock;}
    void setTimeOut(int type, uint32_t t);
    int  getTimeOut(int type);      //0 recv, 1 send
    void setClose() {isClose = true;}
private:
    int mfd;
    int mRecvTimeOut;
    int mSendTimeOut;
    bool isClose;
    bool isSocket;
    bool isNonBlock;
};

class FdManager {
public:
    typedef std::shared_ptr<FdManager> ptr;
    void add(int fd);
    void del(int fd);
    FdCtx::ptr get(int fd);
    static FdManager::ptr& getInstance();
    static FdManager::ptr fdManager;
private:
    FdManager() = default;
    Mutex mLock;
    std::unordered_map<int, FdCtx::ptr> mCtxs;
};


}


#endif