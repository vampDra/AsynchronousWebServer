#include "IOManager.h"
#include <unistd.h>

server::Logger::ptr logger = GET_LOG_INSTANCE;

void handleClient(int fd) {
    while(1) {
        char buf[1024];
        memset(buf, 0, sizeof buf);
        int n = read(fd, buf, sizeof buf);
        if(n == 0) {
            close(fd);
            break;
        }
        cout << buf;
        write(fd, buf, strlen(buf)); 
    }
}

void test_socket() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(10000);
    while(bind(sockfd, (sockaddr*)&addr, sizeof addr) != 0) {
        LOG_ERROR(GET_LOG_INSTANCE) << "bind error " << strerror(errno);
        sleep(2);
    }
    listen(sockfd, 128);
    while(1) {
        int fd = accept(sockfd, nullptr, nullptr);
        server::IOManager::getCurIOManager()->addTask(std::bind(handleClient, fd));
    }
}

void test() {
    server::IOManager::getCurIOManager()->addTask([](){
        sleep(2);
        LOG_INFO(logger) << "sleep 2";
    });
    server::IOManager::getCurIOManager()->addTask([](){
        sleep(3);
        LOG_INFO(logger) << "sleep 3";
        server::IOManager::getCurIOManager()->addTask(test_socket);
    });
    LOG_INFO(logger) << "sleep ";
}

int main() {
    logger->addAppender("stdout");
    logger->setAppenderLevel("stdout", server::LogLevel::INFO);
    server::IOManager iom(1, false, 60, 200);
    iom.addTask(test);
    iom.stop();
}