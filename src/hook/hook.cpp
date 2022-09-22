#include "hook.h"


namespace server {

#define RECV 0
#define SEND 1

#define HOOK_FUN(XX) \
    XX(sleep) \
    XX(socket) \
    XX(connect) \
    XX(accept) \
    XX(read) \
    XX(readv) \
    XX(recv) \
    XX(recvfrom) \
    XX(recvmsg) \
    XX(write) \
    XX(writev) \
    XX(send) \
    XX(sendto) \
    XX(sendmsg) \
    XX(close) 

void hook_init() {
    static bool is_inited = false;
    if(is_inited) {
        return;
    }
#define XX(name) name ## _f = (name ## _fun)dlsym(RTLD_NEXT, #name);
    HOOK_FUN(XX);
#undef XX
}

struct _Hook_Init {
    _Hook_Init() {
        hook_init();
    }
};
static _Hook_Init _hook_init;

}

template<typename OriginFun, typename... Args>
static ssize_t io_func(int fd, uint32_t event, OriginFun func, int type, Args&&... args) 
{
    //处理特殊情况，不存在or不是socket
    server::FdCtx::ptr ctx = server::FdManager::getInstance()->get(fd);
    if(!ctx || !ctx->IsSocket()) {
        return func(fd, std::forward<Args>(args)...);
    }
    if(ctx->IsClose()) {
        errno = EBADF;
        return -1;
    }
Retry:
    ssize_t n = func(fd, std::forward<Args>(args)...);
    //读到数据则返回，否则通过定时器 and 协程异步挂起
    while(n == -1 && errno == EINTR) {
        n = func(fd, std::forward<Args>(args)...);
    }
    //内核无数据，挂起等待响应or定时器到期
    if(n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
        int cancel = 0;
        int to = ctx->getTimeOut(type);
        server::IOManager* iom = server::IOManager::getCurIOManager();
        server::Timer::ptr timer;
        //有超时时间，则添加定时器，定时器到期还无数据到来则关闭连接
        if(to != -1) {
            timer = iom->addTimer(to * 1000, [&cancel, iom, fd, event](){
                cancel = 1;
                iom->cancelEvent(fd, event);
            });
        }
        iom->addEvent(fd, event);
        //让出协程执行
        server::Fiber::getCurFiber()->yield();
        if(timer) {
            iom->delTimer(timer);
        }
        if(cancel) {
            errno = ETIMEDOUT;
            return -1;
        }
        goto Retry;
    }   
    return n;
}


extern "C" {

#define XX(name) name ## _fun name ## _f = nullptr;
    HOOK_FUN(XX);
#undef XX

unsigned int sleep(unsigned int seconds) {
    server::Fiber::ptr fiber = server::Fiber::getCurFiber();
    server::IOManager* iom = server::IOManager::getCurIOManager();
    iom->addTimer(seconds * 1000, std::bind((void(server::Scheduler::*)(server::Fiber::ptr, int thread))&server::IOManager::addTask, iom, fiber, -1));
    server::Fiber::getCurFiber()->yield();
    return 0;
}

int socket(int domain, int type, int protocol) {
    int fd = socket_f(domain, type, protocol);
    if(fd == -1) {
        return fd;
    }
    server::FdManager::getInstance()->add(fd);
    return fd;
}

int accept(int s, struct sockaddr *addr, socklen_t *addrlen) {
    int fd = io_func(s, EPOLLIN, accept_f, RECV, addr, addrlen);
    if(fd >= 0) {
        server::FdManager::getInstance()->add(fd);
    }
    return fd;
}


//读写函数
ssize_t read(int fd, void *buf, size_t count) {
    return io_func(fd, EPOLLIN, read_f, RECV, buf, count);
}

ssize_t readv(int fd, const struct iovec *iov, int iovcnt) {
    return io_func(fd, EPOLLIN, readv_f, RECV, iov, iovcnt);
}

ssize_t recv(int sockfd, void *buf, size_t len, int flags) {
    return io_func(sockfd, EPOLLIN, recv_f, RECV, buf, len, flags);
}

ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen) {
        return io_func(sockfd, EPOLLIN, recvfrom_f, RECV, buf, len, flags, src_addr, addrlen);
}

ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags) {
        return io_func(sockfd, EPOLLIN, recvmsg_f, RECV, msg, flags);
}

ssize_t write(int fd, const void *buf, size_t count) {
    return io_func(fd, EPOLLOUT, write_f, SEND, buf, count);
}

ssize_t writev(int fd, const struct iovec *iov, int iovcnt) {
    return io_func(fd, EPOLLOUT, writev_f, SEND, iov, iovcnt);
}

ssize_t send(int s, const void *msg, size_t len, int flags) {
    return io_func(s, EPOLLOUT, send_f, SEND, msg, len, flags);
}

ssize_t sendto(int s, const void *msg, size_t len, int flags, const struct sockaddr *to, socklen_t tolen) {
    return io_func(s, EPOLLOUT, sendto_f, SEND, msg, len, flags, to, tolen);
}

ssize_t sendmsg(int s, const struct msghdr *msg, int flags) {
    return io_func(s, EPOLLOUT, sendmsg_f, SEND, msg, flags);
}

int close(int fd) {
    server::FdCtx::ptr ctx = server::FdManager::getInstance()->get(fd);
    if(ctx) {
        server::IOManager::getCurIOManager()->delEvent(fd);
        server::FdManager::getInstance()->del(fd);
    }
    return close_f(fd);
}

}