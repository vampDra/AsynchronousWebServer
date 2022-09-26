#include "tcp_server.h"

server::Logger::ptr log = GET_LOG_INSTANCE;
int thread_cnt = 8;

void test() {
    server::TcpServer::ptr server(new server::TcpServer);
    server::Address::ptr addr(new server::Address("0", 10000));
    server->bind(addr);
    server->start();
}

int main() {
    log->addAppender("stdout");
    log->setAppenderLevel("stdout", server::LogLevel::INFO);
    server::IOManager iom(thread_cnt);
    iom.addTask(test);
}