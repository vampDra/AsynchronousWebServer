#include "socket.h"
#include <cstring>

namespace server {

Socket::Socket(Family ipverson, Type type) {
    mSockfd = ::socket(ipverson, type, 0);
}

Socket::Socket(int fd) {
    mSockfd = fd;
    isConnected = true;
}

Socket::~Socket() {
    mSockfd = -1;
    isConnected = false;
}

Socket::ptr Socket::accept(Address::ptr addr) {
    int sock = ::accept(mSockfd, nullptr, nullptr);
    if(sock == -1) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            LOG_ERROR(GET_LOG_INSTANCE) << "accept error, errno=" 
             << errno << ", " << strerror(errno);
        }
        return nullptr;
    } 
    return Socket::ptr(new Socket(sock));
}

int Socket::connect(Address::ptr &addr) {
    isConnected = true;
    return ::connect(mSockfd, addr->getAddr(), addr->getAddrLen());
}

int Socket::bind(Address::ptr &addr) {
    mAddr = addr;
    return ::bind(mSockfd, addr->getAddr(), addr->getAddrLen());
}

int Socket::close() {
    isConnected = false;
    return ::close(mSockfd);
}

ssize_t Socket::send(const char *buf, int size, int flag) {
    return ::send(mSockfd, buf, size, flag);
}

ssize_t Socket::recv(char *buf, int size, int flag) {
    return ::recv(mSockfd, buf, size, flag);
}

Socket::ptr Socket::createTcp() {
    return Socket::ptr(new Socket(IPv4, TCP));
}

Socket::ptr Socket::createUdp() {
    return Socket::ptr (new Socket(IPv4, UDP));
}


}