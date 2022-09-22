#ifndef __HTTP_H__
#define __HTTP_H__
#include "http_define.h"
#include "logger.h"
#include <iostream>
#include <memory>
#include <string>
#include <cstring>
#include <vector>
#include <map>
#include <sstream>

namespace server {

namespace http {

enum class HttpMethod {
#define XX(num, name, des) name = num,
    HTTP_METHOD_MAP(XX)
#undef XX
    INVALID_METHOD
};


enum class HttpStatus {
#define XX(num, name, des) name = num,
    HTTP_STATUS_MAP(XX)
#undef XX
};


//字符串转Http方法
HttpMethod stringToHttpMethod(string s);
//http方法转字符串
const char* httpMethodToString(const HttpMethod& method);
//http状态转字符串
const char* httpStatusToString(const HttpStatus& s);


struct CaseInsensitiveLess {
    bool operator()(const string& lhs, const string& rhs) const;
};


class HttpRequest {
public:
    typedef std::map<string, string, CaseInsensitiveLess> MSS;
    typedef std::shared_ptr<HttpRequest> ptr;
    HttpRequest(string version = "1.1", bool keepAlive = false);

    bool isKeepAlive() const {return mKeepAlive;}
    bool isWebSocket() const {return mWebSocket;}
    HttpMethod getMethod() const {return mMethod;}
    const string &getQuery() const { return mQuery;}
    const string &getPath () const {return mPath;}
    const string &getFrag () const {return mFrag;}
    const string &getBody () const {return mBody;}
    const string &getVersion() const {return mVersion;}
    const string &getTmpHeader() const {return mTmpHeader;}

    void setVersion(string version) {mVersion = version;}
    void setMethod (HttpMethod method) {mMethod = method;}
    void setQuery  (string query) {mQuery = query;}
    void setPath   (string path ) {mPath = path;}
    void setFrag   (string frag ) {mFrag = frag;}
    void setBody   (string body ) {mBody += body;}
    void setKeepAlive(bool keepAlive) {mKeepAlive = keepAlive;}
    void setWebSocket(bool webSocket) {mWebSocket = webSocket;}
    void setTmpHeader(string tmpheader) {mTmpHeader = tmpheader;}
    //获取value
    string getHeader(string key, string def = "") const;
    string getCookie(string key, string def = "") const;
    string getParam (string key, string def = "") const;
    //添加key-value
    void addHeader(string key, string value);
    void addCookie(string key, string value);
    void addParam (string key, string value);
    //删除key
    void delHeader(string key);
    void delCookie(string key);
    void delParam (string key);
    //输出请求报文格式
    string toString();             
private:
    HttpMethod mMethod;     //请求方法
    string mTmpHeader;      //因为解析器的原因，暂存header;
    string mVersion;        //协议版本  1.1/2.0
    string mQuery;          //请求消息
    string mPath;           //文件路径
    string mFrag;           
    string mBody;           //请求消息体
    bool mKeepAlive;        //是否长连接
    bool mWebSocket;        //是否是websocket协议
    MSS mHeaders;           //请求头部
    MSS mCookies;           //cookie
    MSS mParams;            //请求参数
};


class HttpResponse {
public:
    typedef std::shared_ptr<HttpResponse> ptr;
    typedef std::map<string, string, CaseInsensitiveLess> MSS;
    HttpResponse(string version = "1.1", bool keepAlive = false);

    HttpStatus getStatus() const {return mStatus;}
    string getTmpHeader () const {return mTmpHeader;}
    string getVersion   () const {return mVersion;}
    string getBody() const {return mBody;}
    string getHeader(string key, string def = "");
    bool isKeepAlive() {return mKeepAlive;}
    bool isWebSocket() {return mWebSocket;}

    void setWebSocket(bool ws) {mWebSocket = ws;}
    void setKeepAlive(bool keepAlive  ) {mKeepAlive = keepAlive;}
    void setTmpHeader(string tmpheader) {mTmpHeader = tmpheader;}
    void setVersion  (string version  ) {mVersion = version;}
    void setStatus   (HttpStatus status){mStatus = status;}
    void setBody  (string body)  {mBody += body;}
    void addHeader(string key, string value);
    //格式化输出
    string toString();
private:
    std::vector<string> m_cookies;
    HttpStatus mStatus;  //状态码
    string mBody;        //请求体
    string mVersion;     //协议
    string mTmpHeader;   //同上
    bool mKeepAlive;
    bool mWebSocket;
    MSS  mHeaders;       //请求头
};


}
}

#endif //SERVER_HTTP_H
