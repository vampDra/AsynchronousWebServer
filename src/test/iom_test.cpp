#include "IOManager.h"
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

server::Logger::ptr& logger = GET_LOG_INSTANCE;

void handleRead(int fd) {
    while(1) {
        char buf[3];
        memset(buf, 0, sizeof buf);
        int len = read(fd, buf, sizeof buf);
        if(len == 0) {
            close(fd);
            break;
        }
        LOG_INFO(logger) << string(buf, len);
    }
}

void handleAccept(int sockfd) {
    LOG_INFO(logger) << "new connection comming";
    int fd = accept(sockfd, nullptr, nullptr);
    server::IOManager::getCurIOManager()->addEvent(fd, EPOLLIN, std::bind(handleRead, fd));
}

void test1() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(10000);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(sockfd, (sockaddr*)&addr, sizeof addr) != 0) {
        LOG_ERROR(logger) << "bind error  " << errno;
    }
    listen(sockfd, 128);
    server::IOManager::getCurIOManager()->addEvent(sockfd, EPOLLIN, std::bind(handleAccept, sockfd));
}

void test() {
    // logger->addAppender("stdout");
    // logger->setAppenderLevel("stdout", server::LogLevel::INFO);
    server::IOManager::ptr iom(new server::IOManager(2));
    iom->addTask(std::bind(test1));
}

int main() {
    test();
}