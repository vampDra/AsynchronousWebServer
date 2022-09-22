#ifndef __HTTP11_PARSER_H__
#define __HTTP11_PARSER_H__
#include "logger.h"
#include "http_parser.h"
#include "http.h"
#include <memory>
#include <string>


namespace server {
namespace http {

//解析请求回调函数
int request_url_callback   (http_parser *parser, const char *buf, size_t length);
int request_body_callback  (http_parser *parser, const char *buf, size_t length);
int request_header_field_callback (http_parser *parser, const char *buf, size_t length);
int request_header_value_callback (http_parser *parser, const char *buf, size_t length);
int request_header_complete(http_parser *parser);
int request_body_complete  (http_parser *parser);

//解析响应回调函数
int response_body_callback  (http_parser *parser, const char *buf, size_t length);
int response_header_field_callback(http_parser *parser, const char *buf, size_t length);
int response_header_value_callback(http_parser *parser, const char *buf, size_t length);
int response_header_complete(http_parser *parser);
int response_body_complete(http_parser *parser);

//请求解析
class HttpRequestParser {
public:
    typedef std::shared_ptr<HttpRequestParser> ptr;
    HttpRequestParser ();
    ~HttpRequestParser();
    size_t execute(const char *buf, size_t len);
    http_parser *getParser() const { return mParser; }
    HttpRequest::ptr getData() const { return mData; }
    void setFinish(bool finish) {isFinish = finish;}
    bool IsFinish() {return isFinish;}
private:
    http_parser_settings *mSetting; //设置回调
    HttpRequest::ptr mData;         //数据
    http_parser *mParser;           //解析器
    bool isFinish = false;
};


//响应解析
class HttpResponseParser {
public:
    typedef std::shared_ptr<HttpResponseParser> ptr;
    HttpResponseParser ();
    ~HttpResponseParser();
    size_t execute(const char *buf, size_t len);
    http_parser *getParser () const {return mParser;}
    HttpResponse::ptr getData() const {return mData;}
    void setFinish(bool finish) {isFinish = finish;}
    bool IsFinish() {return isFinish;}
private:
    http_parser_settings *mSetting; //设置回调
    HttpResponse::ptr mData;        //数据
    http_parser *mParser;           //解析器
    bool isFinish = false;
};


}
}


#endif