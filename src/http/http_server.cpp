#include "http_server.h"


namespace server {
namespace http {

Logger::ptr logger = GET_LOG_INSTANCE;

HttpServer::HttpServer(int thread_cnt, IOManager *accept, IOManager *worker)
: TcpServer(thread_cnt, accept, worker) {
    mDispatch.reset(new ServletDispatch);
}

void HttpServer::handleClient(Socket::ptr sock) {
    while(true) {
        HttpRequestParser::ptr req_parser(new HttpRequestParser);
        //返回: 读到解析完成 || 对端断开连接 || 出错
        while(true) {
            char buf[1024];
            memset(buf, 0, sizeof buf);
            ssize_t n = sock->recv(buf, sizeof buf);
            if(n <= 0) {
                sock->close();
                return;
            }
            size_t t = req_parser->execute(buf, strlen(buf));
            if(t != strlen(buf)) {
                LOG_WARNING(logger) << "req parser error, quit";
                sock->close();
                return;
            }
            if(req_parser->IsFinish()) {
                break;
            }
        }
        HttpRequest::ptr req = req_parser->getData();
        HttpResponse::ptr res(new HttpResponse(req->getVersion(), req->isKeepAlive()));
        mDispatch->handle(req, res);
        string send_buf = res->toString();
        size_t offset = 0;
        while(offset != send_buf.size()) {
            ssize_t n = sock->send(send_buf.c_str() + offset, send_buf.size() - offset);
            offset += n;
        }
        if(!sock->isConnect() || !req->isKeepAlive()) {
            sock->close();
            break;
        }
    }
}

}
}