#include"http11_parser.h"


namespace server {
namespace http {

static Logger::ptr httpLogger = GET_LOG_INSTANCE;

int request_url_callback(http_parser *parser, const char *buf, size_t length) {
    HttpRequestParser *req = (HttpRequestParser *)parser->data;
    HttpRequest::ptr data = req->getData();
    data->setPath(string(buf, length));
    return 0;
}

int request_body_callback(http_parser *parser, const char *buf, size_t length) {
    HttpRequestParser *req = (HttpRequestParser *)parser->data;
    HttpRequest::ptr data = req->getData();
    data->setBody(string(buf, length));
    return 0;
}

int request_header_field_callback(http_parser *parser, const char *buf, size_t length) {
    HttpRequestParser *req = (HttpRequestParser *)parser->data;
    HttpRequest::ptr data = req->getData();
    data->setTmpHeader(string(buf, length));   //暂存key，等待value回调
    return 0;
}

int request_header_value_callback(http_parser *parser, const char *buf, size_t length) {
    HttpRequestParser *req = (HttpRequestParser *)parser->data;
    HttpRequest::ptr data = req->getData();
    string key = data->getTmpHeader();
    string val = string(buf, length);
    if(strcasecmp(key.c_str(), "connection") == 0) {
        data->setKeepAlive((strcasecmp(val.c_str(), "close") == 0 ? false : true));
    }
    data->addHeader(key, val);
    return 0;
}

int request_header_complete(http_parser *parser) {
    HttpRequestParser *req = (HttpRequestParser *)parser->data;
    HttpRequest::ptr data = req->getData();
    data->setMethod((HttpMethod)parser->method);
    data->setVersion(std::to_string(parser->http_major) + "." + std::to_string(parser->http_minor));
    return 0;
}

int request_body_complete(http_parser *parser) {
    HttpRequestParser *req = (HttpRequestParser *)parser->data;
    req->setFinish(true);
    return 0;
}

HttpRequestParser::HttpRequestParser() {
    //初始化
    mData.reset(new HttpRequest());
    mSetting = new http_parser_settings;
    mParser  = new http_parser;
    http_parser_settings_init(mSetting);
    http_parser_init(mParser, HTTP_REQUEST);
    //注册回调
    mSetting->on_url = request_url_callback;
    mSetting->on_body = request_body_callback;
    mSetting->on_header_field = request_header_field_callback;
    mSetting->on_header_value = request_header_value_callback;
    mSetting->on_headers_complete = request_header_complete;
    mSetting->on_message_complete = request_body_complete;
    mParser->data = this;
}

HttpRequestParser::~HttpRequestParser() {
    delete mSetting;
    delete mParser;
    mSetting = nullptr, mParser = nullptr;
}

size_t HttpRequestParser::execute(const char *buf, size_t len) {
    size_t offset = http_parser_execute(mParser, mSetting, buf, len);
    if(offset != len) {
        LOG_WARNING(httpLogger) << "request parser error:"
        << http_errno_name((http_errno)mParser->http_errno)
        << ", " << http_errno_description((http_errno)mParser->http_errno);
    }
    return offset;
}


int response_body_callback(http_parser * parser, const char *buf, size_t length) {
    HttpResponseParser *req = (HttpResponseParser *)parser->data;
    HttpResponse::ptr data = req->getData();
    data->setBody(string(buf, length));
    return 0;
}

int response_header_complete(http_parser * parser) {
    HttpResponseParser *req = (HttpResponseParser *)parser->data;
    HttpResponse::ptr data = req->getData();
    data->setVersion(std::to_string(parser->http_major) + "." + std::to_string(parser->http_minor));
    data->setStatus((HttpStatus)parser->state);
    return 0;
}

int response_body_complete(http_parser *parser) {
    HttpResponseParser *req = (HttpResponseParser *)parser->data;
    req->setFinish(true);
    return 0;
}

int response_header_field_callback(http_parser * parser, const char *buf, size_t length) {
    HttpResponseParser *req = (HttpResponseParser *)parser->data;
    HttpResponse::ptr data = req->getData();
    data->setTmpHeader(string(buf, length));
    return 0;
}

int response_header_value_callback(http_parser * parser, const char *buf, size_t length) {
    HttpResponseParser *req = (HttpResponseParser *)parser->data;
    HttpResponse::ptr data = req->getData();
    string key = data->getTmpHeader();
    string val = string(buf, length);
    if(strncasecmp("connection", key.c_str(), strlen("connection")) == 0) {
        data->setKeepAlive((strncasecmp("close", val.c_str(), strlen("close")) == 0 ? false : true));
    }
    data->addHeader(key, val);
    return 0;
}


HttpResponseParser::HttpResponseParser() {
    mSetting  = new http_parser_settings;
    mParser   = new http_parser;
    mData.reset(new HttpResponse());
    http_parser_init(mParser, HTTP_RESPONSE);
    http_parser_settings_init(mSetting);

    mSetting->on_body = response_body_callback;
    mSetting->on_header_field = response_header_field_callback;
    mSetting->on_header_value = response_header_value_callback;
    mSetting->on_headers_complete = response_header_complete;
    mParser->data = this;
}

HttpResponseParser::~HttpResponseParser() {
    delete mSetting;
    delete mParser;
    mSetting = nullptr, mParser = nullptr;
}

size_t HttpResponseParser::execute(const char *buf, size_t len) {
    size_t offset = http_parser_execute(mParser, mSetting, buf, len);
    if (offset != len) {
        LOG_WARNING(httpLogger) << "response parser error:"
            << http_errno_name((http_errno)mParser->http_errno)
            << ", " << http_errno_description((http_errno)mParser->http_errno);
    }
    return offset;
}


}
}