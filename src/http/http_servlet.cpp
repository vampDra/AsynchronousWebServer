#include "http_servlet.h"


namespace server {
namespace http {

ServletDispatch::ServletDispatch() {
    mNotFound = std::make_shared<NotFoundServlet>();
}

int ServletDispatch::handle(HttpRequest::ptr req, HttpResponse::ptr res) {
    Servlet::ptr servlet = getServlet(req->getPath());
    return servlet->handle(req, res);
} 

void ServletDispatch::addServlet(string uri, FuncServlet::callback cb) {
    mDatas[uri] = std::make_shared<FuncServlet>(cb);
}

Servlet::ptr ServletDispatch::getServlet(string uri) {
    auto it = mDatas.find(uri);
    if(it != mDatas.end()) {
        return it->second;
    }
    return mNotFound;
}

int NotFoundServlet::handle(HttpRequest::ptr req, HttpResponse::ptr res) {
    res->setStatus((HttpStatus)404);
    res->setVersion(req->getVersion());
    res->setKeepAlive(req->isKeepAlive());
    string body =
        "<html><head><title>404 Not Found"
        "</title></head><body><center><h1>404 Not Found</h1></center>"
        "<hr><center>Jerry Han</center></body></html>";
    res->setBody(body);
    res->addHeader("content-length", std::to_string(body.size()));
    return 0;
}


}
}