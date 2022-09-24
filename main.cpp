#include "http_server.h"
#include <unistd.h>

server::Logger::ptr log = GET_LOG_INSTANCE;

int port = 10000;
void test() {
    server::http::HttpServer::ptr http_server(new server::http::HttpServer);
    server::Address::ptr addr(new server::Address("0", port));
    http_server->bind(addr);
    http_server->start();
}

int main(int argc, char **argv) {
    if(argc >= 2) {
        port = atoi(argv[1]);
    }
    
    log->addAppender("stdout", server::LogLevel::INFO);
    server::IOManager iom(2);
    iom.addTask(test);
}
