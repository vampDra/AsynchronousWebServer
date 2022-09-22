#include "http.h"

namespace server {

namespace http {

HttpMethod stringToHttpMethod(string s) {
#define XX(num, method, des) \
if(s == #method) { \
    return HttpMethod::method; \
}
    HTTP_METHOD_MAP(XX)
#undef XX
    return HttpMethod::INVALID_METHOD;
}

const char* httpMethodToString(const HttpMethod& method) {
    switch (method) {
#define XX(num, name, des) \
    case HttpMethod::name: \
        return #name;

        HTTP_METHOD_MAP(XX)
        case HttpMethod::INVALID_METHOD:
            return "invalid_method";
    }
#undef XX
}

const char* httpStatusToString(const HttpStatus& s) {
    switch(s) {
#define XX(code, name, msg) \
    case HttpStatus::name: \
        return #msg;
        HTTP_STATUS_MAP(XX)
#undef XX
        default:
            return "<unknown>";
    }
}

bool CaseInsensitiveLess::operator()(const string &lhs, const string &rhs) const{
    return strcasecmp(lhs.c_str(), rhs.c_str()) < 0;
}



HttpRequest::HttpRequest(string version, bool keepAlive)
: mMethod(HttpMethod::GET)
, mVersion(version)
, mPath("/")
, mKeepAlive(keepAlive)
, mWebSocket(false) {}

string HttpRequest::getHeader(string key, string def) const {
    auto it = mHeaders.find(key);
    return it == mHeaders.end() ? def : it->second;
}

string HttpRequest::getParam(string key, string def) const {
    auto it = mParams.find(key);
    return it == mParams.end() ? def : it->second;
}

string HttpRequest::getCookie(string key, string def) const {
    auto it = mCookies.find(key);
    return it == mCookies.end() ? def : it->second;
}

void HttpRequest::addHeader(string key, string value) {
    mHeaders[key] = value;
}

void HttpRequest::addParam(string key, string value) {
    mParams[key] = value;
}

void HttpRequest::addCookie(string key, string value) {
    mCookies[key] = value;
}

void HttpRequest::delHeader(string key) {
    mHeaders.erase(key);
}

void HttpRequest::delParam(string key) {
    mParams.erase(key);
}

void HttpRequest::delCookie(string key) {
    mCookies.erase(key);
}

string HttpRequest::toString() {
    std::stringstream ss;
    ss << httpMethodToString(mMethod) << " "
       << mPath << " "
       << (mQuery.empty() ? "" : "?")
       << mQuery
       << (mFrag.empty() ? "" : "#")
       << mFrag
       << "HTTP/"
       << mVersion
       << "\r\n";

    if(!mWebSocket) {
        ss << "connection:" << (mKeepAlive ? "keep-alive" : "close") << "\r\n";
    }
    for(auto it : mHeaders) {
        if(!mWebSocket && strcasecmp("connection", it.first.c_str()) == 0) {
            continue;
        }
        ss << it.first << ":" << it.second << "\r\n";
    }
    if(!mBody.empty()) {
        if(mHeaders.find("content-length") == mHeaders.end()) {
            ss << "content-length:" << mBody.size() << "\r\n";
        }
       ss << "\r\n" << mBody;
    } else {
        ss << "\r\n";
    }
    return ss.str();
}


HttpResponse::HttpResponse(string version, bool keepAlive)
: mStatus(HttpStatus::OK)
, mVersion(version)
, mKeepAlive(keepAlive)
, mWebSocket(false) {}

string HttpResponse::getHeader(string key, string def) {
    auto it = mHeaders.find(key);
    return it == mHeaders.end() ? def : it->second;
}

void HttpResponse::addHeader(string key, string value) {
    mHeaders[key] = value;
}

string HttpResponse::toString() {
    std::stringstream ss;
    ss << "HTTP/"
       << mVersion
       << " "
       << (int)mStatus
       << " "
       << httpStatusToString(mStatus)
       << "\r\n";

    if(!mWebSocket) {
        ss << "connection:" << (mKeepAlive ? "keep-alive" : "close") << "\r\n";
    }
    for(auto it : mHeaders) {
        if(!mWebSocket && strcasecmp(it.first.c_str(), "connection") == 0) {
            continue;
        }
        ss << it.first << ":" << it.second << "\r\n";
    }
    if(mBody.size()) {
        if(mHeaders.find("content-length") == mHeaders.end()) {
            ss << "content-length:" << mBody.size() << "\r\n";
        }
        ss << "\r\n" << mBody;
    } else {
        ss << "\r\n";
    }
    return ss.str();
}


}
}