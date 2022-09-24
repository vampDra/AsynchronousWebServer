#ifndef __HTTP_SERVLET_H__
#define __HTTP_SERVLET_H__
#include "http11_parser.h"
#include "http.h"
#include <memory>
#include <functional>
#include <unordered_map>


namespace server {
namespace http {
/**
 * @brief servlet为uri执行基类，通过ServletDispatch统一管理
 * 
 */
class Servlet {
public:
    typedef std::shared_ptr<Servlet> ptr;
    Servlet() = default;
    virtual ~Servlet() = default;
    virtual int handle(HttpRequest::ptr req, HttpResponse::ptr res) = 0;
};

class FuncServlet : public Servlet {
public:
    typedef std::shared_ptr<FuncServlet> ptr;
    typedef std::function<int(HttpRequest::ptr, HttpResponse::ptr)> callback;
    FuncServlet () = default;
    ~FuncServlet() = default;
    FuncServlet(callback cb) { mCB = cb; }
    int handle(HttpRequest::ptr req, HttpResponse::ptr res) {return mCB(req, res);}
private : 
    callback mCB;
};

class ServletDispatch : public Servlet{
public:
    typedef std::shared_ptr<ServletDispatch> ptr;
    ServletDispatch ();
    ~ServletDispatch() = default;
    int handle(HttpRequest::ptr req, HttpResponse::ptr res) override;
    void addServlet(string uri, FuncServlet::callback cb);
    void addServlet(string uri, Servlet::ptr slt);
    Servlet::ptr getServlet(string uri);
private:
    std::unordered_map<string, Servlet::ptr> mDatas;
    Servlet::ptr mNotFound;
};

class NotFoundServlet : public Servlet {
public:
    typedef std::shared_ptr<NotFoundServlet> ptr;
    int handle(HttpRequest::ptr req, HttpResponse::ptr res) override;
};

class DefaultServlet : public Servlet {
public:
    typedef std::shared_ptr<NotFoundServlet> ptr;
    int handle(HttpRequest::ptr req, HttpResponse::ptr res) override;
};

}
}

#endif