#include "http_server.h"
#include <unistd.h>

int thread_cnt = 6;
server::Logger::ptr log = GET_LOG_INSTANCE;
void test() {
    server::http::HttpServer::ptr server(new server::http::HttpServer);
    server::Address::ptr addr(new server::Address("0", 10000));
    server->bind(addr);
    server->start();
}

int main() {
    log->addAppender("stdout");
    log->setAppenderLevel("stdout", server::LogLevel::INFO);
    server::IOManager iom(thread_cnt, false);
    iom.addTask(test);
}
