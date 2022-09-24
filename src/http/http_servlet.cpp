#include "http_servlet.h"


namespace server {
namespace http {

ServletDispatch::ServletDispatch() {
    mNotFound.reset(new NotFoundServlet);
    mDatas["/"] = std::make_shared<DefaultServlet>();
}

int ServletDispatch::handle(HttpRequest::ptr req, HttpResponse::ptr res) {
    Servlet::ptr servlet = getServlet(req->getPath());
    return servlet->handle(req, res);
} 

void ServletDispatch::addServlet(string uri, FuncServlet::callback cb) {
    mDatas[uri] = std::make_shared<FuncServlet>(cb);
}

void ServletDispatch::addServlet(string uri, Servlet::ptr slt) {
    mDatas[uri] = slt;
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

int DefaultServlet::handle(HttpRequest::ptr req, HttpResponse::ptr res) {
    res->setStatus((HttpStatus)200);
    res->setVersion(req->getVersion());
    res->setKeepAlive(req->isKeepAlive());
    string body = 
        "<html><head><title>Jerry han's Server"
        "</title></head><body><center><h1>Welcome to JerryHan's server</h1></center>"
        "<hr><center>Jerry Han</center></body></html>";
    res->setBody(body);
    res->addHeader("content-length", std::to_string(body.size()));
    res->addHeader("content-type", "text/html");
    return 0;
}

}
}