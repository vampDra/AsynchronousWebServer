#ifndef __LOGGER_H__
#define __LOGGER_H__
#include "lock.h"
#include <fstream>
#include <functional>
#include <iostream>
#include <list>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>
using std::string;
using std::cin;
using std::cout;
using std::endl;


//单例模式获取实例
#define GET_LOG_INSTANCE server::Logger::getInstance()

#define LOG(logger, level)                                           \
    if (level >= logger->getLevel())                                 \
    server::LogEventWrap(logger, level,                              \
                         server::LogEvent::ptr(new server::LogEvent( \
                             __FILE__, __LINE__, time(nullptr)))).getSS()

#define LOG_DEBUG(logger) LOG(logger, server::LogLevel::DEBUG)
#define LOG_INFO(logger) LOG(logger, server::LogLevel::INFO)
#define LOG_WARNING(logger) LOG(logger, server::LogLevel::WARNING)
#define LOG_ERROR(logger) LOG(logger, server::LogLevel::ERROR)
#define LOG_FATAL(logger) LOG(logger, server::LogLevel::FATAL)


namespace server {

class LogLevel {
public:
    enum Level { DEBUG = 0, INFO, WARNING, ERROR, FATAL };
    static const char* ToString(LogLevel::Level level);
    static LogLevel::Level FromString(const string str);
};

//产生日志事件
class LogEvent {
public:
    typedef std::shared_ptr<LogEvent> ptr;
    LogEvent(const char* filename, int line, time_t time) 
    :mFileName(filename), mLineNum(line), mTime(time) {}
    int getLineNum() const { return mLineNum; };
    time_t getTime() const { return mTime; };
    string getContent() const { return ss.str(); };
    std::stringstream& getSS() { return ss; };
    const char* getFileName () const { return mFileName; };
private:
    const char* mFileName;      //产生日志的文件名，可以通过__FILE__宏来赋值
    std::stringstream ss;       //内容被写入ss中
    int mLineNum;               //行号 __LINE__
    time_t mTime;               //时间戳
};

/*
* %m 消息
* %p 日志级别
* %d 日期时间，后面可跟一对括号指定时间格式，比如%d{%Y-%m-%d %H:%M:%S}
* %f 文件名
* %l 行号
* %% 百分号
* %T 制表符
* %n 换行
* 
*/
class LogFormatter {
public:
    class FormatItem {
    public:
        typedef std::shared_ptr<FormatItem> ptr;
        virtual ~FormatItem() = default;
        virtual void format(std::stringstream& os, LogLevel::Level level, LogEvent::ptr event) = 0;
    };
public:
    typedef std::shared_ptr<LogFormatter> ptr;
    LogFormatter(string pattern);
    void setPattern(const string& pattern);
    string format(LogLevel::Level level, const LogEvent::ptr& event);  //按格式输出日志
private:
    void init();        //解析字符串并存储对应item
    std::vector<FormatItem::ptr> mItem;
    string mPattern;
};

//日志输出地 虚基类
class LogAppender {
public:
    typedef std::shared_ptr<LogAppender> ptr;
    LogAppender(LogLevel::Level level);
    virtual ~LogAppender() = default;
    virtual void log(LogLevel::Level level, LogEvent::ptr event) = 0;   //日志输出虚函数
    void setLevel(LogLevel::Level level) { mLevel = level; }
    void setFormatter(const string format);
    LogLevel::Level getLevel() const {return mLevel;}
    LogFormatter::ptr getFormatter() const {return mFormatter;}
protected:
    LogFormatter::ptr mFormatter;  //调整格式
    LogLevel::Level mLevel;
    string mFileName;
    Mutex  mLock;
};

class StdoutAppender : public LogAppender {
public:
    typedef std::shared_ptr<StdoutAppender> ptr;
    StdoutAppender(LogLevel::Level level = LogLevel::DEBUG);
    void log(LogLevel::Level level, LogEvent::ptr event) override;
};

class FileAppender : public LogAppender {
public:
    FileAppender (string name = "", LogLevel::Level level = LogLevel::DEBUG);
    ~FileAppender() override { os.close(); }
    void log(LogLevel::Level level, LogEvent::ptr event) override;
private:
    std::ofstream os;
};

//日志器
class Logger {
public:
    typedef std::shared_ptr<Logger> ptr;
    static Logger::ptr& getInstance();  //采用单例模式获取实例，比较优雅
    void log(LogLevel::Level level, const LogEvent::ptr& event);
    void setLevel(LogLevel::Level level) {mLevel = level;}
    void addAppender (const string filename, LogLevel::Level level = LogLevel::DEBUG);
    void delAppender (const string filename);
    void setAppenderLevel(const string filename, LogLevel::Level level);
    void setAppenderFormatter(const string filename, const string& format);
    LogLevel::Level getLevel () const {return mLevel;}
private:
    Logger() = default;
    std::unordered_map<string, LogAppender::ptr> mAppends;  //存储输出地
    LogLevel::Level mLevel = LogLevel::DEBUG;
    Mutex mLock;
};

//日志输出管理
class LogEventWrap {
public:
    LogEventWrap(Logger::ptr& log, LogLevel::Level level, LogEvent::ptr event)
    : mEvent(event), mLog(log), mLevel(level) {}
    std::stringstream& getSS() {return mEvent->getSS();}
    ~LogEventWrap() {mLog->log(mLevel, mEvent);}
private:
    LogEvent::ptr   mEvent;
    std::shared_ptr<Logger> mLog;
    LogLevel::Level mLevel;
};


}

#endif