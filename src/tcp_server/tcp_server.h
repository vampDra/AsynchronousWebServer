#ifndef __TCP_SERVER_H__
#define __TCP_SERVER_H__
#include "address.h"
#include "socket.h"
#include "IOManager.h"
#include "logger.h"
#include <functional>
#include <memory>
#include <cstring>


namespace server {
/*
 * 创建TCP连接基类，实现数据回传，简易回声服务器
 */
class TcpServer : 
    public std::enable_shared_from_this<TcpServer> {
public:
    typedef std::shared_ptr<TcpServer> ptr;
    TcpServer (IOManager *accept = IOManager::getCurIOManager(), IOManager *worker = IOManager::getCurIOManager());
    bool bind (Address::ptr& addr);
    void start();
    void stop ();
    virtual ~TcpServer() = default;
protected:
    void handleAccept();
    virtual void handleClient(Socket::ptr sock);
protected:
    Socket::ptr mLisSock;
    IOManager *mAcceptor;
    IOManager *mWorker;
    bool mIsStop;
};


}


#endif //SERVER_TCP_SERVER_H