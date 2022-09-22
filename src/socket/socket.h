#ifndef __SOCKET_H__
#define __SOCKET_H__
#include "address.h"
#include "logger.h"
#include <fcntl.h>
#include <memory>
#include <unistd.h>


namespace server {

class Socket {
public:
    typedef std::shared_ptr<Socket> ptr;
    enum Type {
        TCP = SOCK_STREAM,
        UDP = SOCK_DGRAM
    };
    enum Family {
        IPv4 = AF_INET,
        IPv6 = AF_INET6,
        UNIX = AF_UNIX,
    };
public:
    Socket (Family ipverson, Type type);
    Socket (int fd);       //通过accept接收创建
    Socket () = default;
    ~Socket();

    Address::ptr getLocalAddr() const {return mAddr;}
    bool isConnect() const {return isConnected;}
    int  getSockfd() const {return mSockfd;}
    int  listen()  {return ::listen(mSockfd, 128);}
    //UDP接口待做
    ssize_t sendTo()  {return 0;}
    ssize_t recvFrom(){return 0;}
    ssize_t send (const char *buf, int size, int flag = 0);
    ssize_t recv (char *buf, int size, int flag = 0);

    Socket::ptr accept(Address::ptr addr = nullptr);
    int connect(Address::ptr &addr);
    int bind (Address::ptr &addr);
    int close();
public:
    static Socket::ptr createTcp();
    static Socket::ptr createUdp();
private:
    int  mSockfd = -1;
    bool isConnected = false;
    Address::ptr mAddr;
};



}


#endif //SERVER_SOCKET_H
