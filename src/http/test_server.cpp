#include "test_server.h"
#include <fcntl.h>

namespace server {
namespace http {

TestServer::TestServer(IOManager *accept, IOManager *worker)
: TcpServer(accept, worker) {
    mDispatch.reset(new ServletDispatch);
}

void TestServer::handleClient(Socket::ptr sock) {
    fcntl(sock->getSockfd(), F_SETFL, O_NONBLOCK);
    IOManager::getCurIOManager()->addEvent(sock->getSockfd(), EPOLLIN, std::bind(&TestServer::handleRead,this, sock));
}

void TestServer::handleRead(Socket::ptr sock) {
    char buf[2048];
    memset(buf, 0, sizeof buf);
    int n = sock->recv(buf, sizeof buf);
    if(n == 0) {
        IOManager::getCurIOManager()->delEvent(sock->getSockfd());
        sock->close();
        return;
    }
    HttpRequestParser::ptr parser(new HttpRequestParser);
    parser->execute(buf, strlen(buf));
    auto req = parser->getData();
    HttpResponse::ptr res(new HttpResponse(req->getVersion(), req->isKeepAlive()));
    IOManager::getCurIOManager()->addTask(Fiber::getCurFiber());
    Fiber::getCurFiber()->yield();
    IOManager::getCurIOManager()->addEvent(sock->getSockfd(), EPOLLOUT, std::bind(&TestServer::handleWrite, this, sock));
}

void TestServer::handleWrite(Socket::ptr sock) {
    const char *buf = "HTTP/1.0 200 OK\r\nconnection:keep-alive\r\n\r\n";
    sock->send(buf, strlen(buf));
    IOManager::getCurIOManager()->addEvent(sock->getSockfd(), EPOLLIN, std::bind(&TestServer::handleRead, this, sock));
}

}
}