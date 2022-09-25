#ifndef __HTTP_SERVER_H__
#define __HTTP_SERVER_H__
#include "timer.h"
#include "tcp_server.h"
#include "http11_parser.h"
#include "http.h"
#include "http_servlet.h"
#include <memory>


namespace server {
namespace http {
/**
 * @brief 基于TcpServer实现HttpServer
 * 主要实现重载handleClient与添加servlet的功能。
 * 所有的读写函数均已通过协程hook，所以可以采取同步方式编写
 * 
 */
class HttpServer : public TcpServer {
public:
    typedef std::shared_ptr<HttpServer> ptr;
    HttpServer (int thread_cnt, IOManager *accept = IOManager::getCurIOManager(), IOManager *worker = IOManager::getCurIOManager());
    ~HttpServer() = default;
    void addServlet(std::string uri, FuncServlet::callback cb);
private:
    void handleClient(Socket::ptr sock) override;
private:
    ServletDispatch::ptr mDispatch;
};


}
}

#endif