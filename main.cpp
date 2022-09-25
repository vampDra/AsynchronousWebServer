#include "http_server.h"
#include "config.h"

server::Logger::ptr logger = GET_LOG_INSTANCE;
int port = 10000;
int thread_num = 4;

void test() {
    server::http::HttpServer::ptr http_server(new server::http::HttpServer(thread_num));
    server::Address::ptr addr(new server::Address("0", port));
    http_server->bind(addr);
    http_server->start();
}

int main(int argc, char **argv) {
    Config conf(port, thread_num);
    conf.parse_arg(argc, argv);

    port = conf.mPort;
    thread_num = conf.mThreadCnt;

    logger->addAppender("stdout", server::LogLevel::INFO);
    server::IOManager iom(thread_num);
    iom.addTask(test);
}
