//
// Created by 11465 on 2022/5/30.
//

#include "logger.h"


namespace server {

// Level的转化
const char* LogLevel::ToString(LogLevel::Level level) {
    switch (level) {
#define XX(name)         \
    case LogLevel::name: \
        return #name;    \
        break;

        XX(DEBUG)
        XX(INFO)
        XX(WARNING)
        XX(ERROR)
        XX(FATAL)

#undef XX
        default:
            return "UNKNOWN";
    }
}

LogLevel::Level LogLevel::FromString(const string str) {
#define XX(level, v)            \
    if (str == #v) {            \
        return LogLevel::level; \
    }

    XX(DEBUG, debug)
    XX(INFO, info)
    XX(WARNING, warn)
    XX(ERROR, error)
    XX(FATAL, fatal)

    XX(DEBUG, DEBUG)
    XX(INFO, INFO)
    XX(WARNING, WARN)
    XX(ERROR, ERROR)
    XX(FATAL, FATAL)

#undef XX
    return LogLevel::DEBUG;
}

/*
 * 一堆继承item类
 */
class MessageFormatItem : public LogFormatter::FormatItem {
public:
    explicit MessageFormatItem(const string& str = "") {}
    void format(std::stringstream& os,
                LogLevel::Level level,
                LogEvent::ptr event) override {
        os << event->getContent();
    }
};

class LevelFormatItem : public LogFormatter::FormatItem {
public:
    explicit LevelFormatItem(const string& str = "") {}
    void format(std::stringstream& os,
                LogLevel::Level level,
                LogEvent::ptr event) override {
        os << LogLevel::ToString(level);
    }
};

class FileNameFormatItem : public LogFormatter::FormatItem {
public:
    explicit FileNameFormatItem(const string& str = "") {}
    void format(std::stringstream& os,
                LogLevel::Level level,
                LogEvent::ptr event) override {
        os << event->getFileName();
    }
};

class LineFormatItem : public LogFormatter::FormatItem {
public:
    explicit LineFormatItem(const string& str = "") {}
    void format(std::stringstream& os,
                LogLevel::Level level,
                LogEvent::ptr event) override {
        os << event->getLineNum();
    }
};

class TimeFormatItem : public LogFormatter::FormatItem {
public:
    explicit TimeFormatItem(const string& str) {
        mFormat = str;
        if (mFormat.empty()) {
            mFormat = "%Y-%m-%d %H:%M:%S";
        }
    }
    void format(std::stringstream& os,
                LogLevel::Level level,
                LogEvent::ptr event) override {
        //这个函数能把秒数转换为日期格式
        struct tm tm = {};
        time_t time = event->getTime();
        localtime_r(&time, &tm);
        char buf[64];
        strftime(buf, sizeof(buf), mFormat.c_str(), &tm);
        os << buf;
    }
private:
    string mFormat;
};

class NewLineFormatItem : public LogFormatter::FormatItem {
public:
    explicit NewLineFormatItem(const string& str = "") {}
    void format(std::stringstream& os,
                LogLevel::Level level,
                LogEvent::ptr event) override {
        os << "\n";
    }
};

class TabFormatItem : public LogFormatter::FormatItem {
public:
    explicit TabFormatItem(const string& str = "") {}
    void format(std::stringstream& os,
                LogLevel::Level level,
                LogEvent::ptr event) override {
        os << "\t";
    }
};

class StringFormatItem : public LogFormatter::FormatItem {
public:
    explicit StringFormatItem(string str) : mString(str) {}
    void format(std::stringstream& os,
                LogLevel::Level level,
                LogEvent::ptr event) override {
        os << mString;
    }
private:
    string mString;
};

LogFormatter::LogFormatter(string pattern)
: mPattern(pattern) {
    init();
}

void LogFormatter::init() {
    std::vector<std::tuple<string, string, bool> > vec;
    int n = (int)mPattern.size();
    string normalStr;  //存储非转义字符串
    for (int i = 0; i < n; i++) {
        //存储非转义部分
        if (mPattern[i] != '%') {
            normalStr += mPattern[i];
            continue;
        }
        // 存储%%这种格式
        if (i + 1 < n && mPattern[i + 1] == '%') {
            normalStr += '%';
            continue;
        }

        //从%后面开始解析
        int state = 0;  // 0解析正常转义，1解析{}
        int idx = i + 1;
        int fmtBegin = 0;
        string str;
        string timeFmt;

        while (idx < n) {
            if (!state && !isalpha(mPattern[idx]) && mPattern[idx] != '{') {
                str = mPattern.substr(i + 1, idx - (i + 1));
                break;
            }
            if (!state && mPattern[idx] == '{') {
                str = mPattern.substr(i + 1, idx - (i + 1));
                fmtBegin = idx + 1;
                idx++;
                state = 1;
                continue;
            }
            if (state == 1 && mPattern[idx] == '}') {
                timeFmt = mPattern.substr(fmtBegin, idx - fmtBegin);
                idx++;
                break;
            }
            idx++;
            if (idx == n && str.empty())
                str = mPattern.substr(i + 1);
        }
        i = idx - 1;

        //单次解析完成
        if (!normalStr.empty()) {
            vec.emplace_back(std::make_tuple(normalStr, "", false));
            normalStr.clear();
        }
        vec.emplace_back(std::make_tuple(str, timeFmt, 1));
    }
    if (!normalStr.empty())
        vec.emplace_back(std::make_tuple(normalStr, "", false));

    /*
     * 截止目前，所有转义/非转义和{}已经全部存入vec中，待分配item
     */
    static std::unordered_map<
        string, std::function<FormatItem::ptr(const string& str)> >
        map = {
#define XX(str, C)                                                             \
    {                                                                          \
#str,                                                                  \
            [](const string& fmt) { return FormatItem::ptr(new C(fmt)); } \
    }

            XX(m, MessageFormatItem), XX(p, LevelFormatItem),
            XX(d, TimeFormatItem),    XX(f, FileNameFormatItem),
            XX(l, LineFormatItem),    XX(n, NewLineFormatItem),
            XX(T, TabFormatItem)
#undef XX
        };

    for (auto& i : vec) {
        if (!std::get<2>(i)) {
            mItem.push_back(
                FormatItem::ptr(new StringFormatItem(std::get<0>(i))));
        } else {
            auto it = map.find(std::get<0>(i));
            if (it != map.end()) {
                mItem.push_back(it->second(std::get<1>(i)));
            }
        }
    }
}

void LogFormatter::setPattern(const string& pattern) {
    mPattern = pattern;
    mItem.clear();
    init();
}

string LogFormatter::format(LogLevel::Level level,
                                 const LogEvent::ptr& event) {
    std::stringstream ss;
    for (FormatItem::ptr& it : mItem) {
        it->format(ss, level, event);
    }
    return ss.str();
}

LogAppender::LogAppender(LogLevel::Level level)
: mLevel(level) {
    if (!mFormatter) {
        mFormatter.reset(new LogFormatter("%p    %d{%Y-%m-%d %H:%M:%S}     %f:%l    [%m]%n"));
    }
}

void LogAppender::setFormatter(const string format) {
    LockGuard lock(mLock);
    mFormatter->setPattern(format);
}

StdoutAppender::StdoutAppender(LogLevel::Level level)
: LogAppender(level) {}

void StdoutAppender::log(LogLevel::Level level, LogEvent::ptr event) {
    if (level >= mLevel) {
        LockGuard lock(mLock);
        std::cout << mFormatter->format(level, event);
    }
}

FileAppender::FileAppender(string name, LogLevel::Level level)
: LogAppender(level) {
    if (name.empty())
        mFileName = "log.txt";
    else
        mFileName = name;
}

void FileAppender::log(LogLevel::Level level, LogEvent::ptr event) {
    if (level >= mLevel) {
        LockGuard lock(mLock);
        os.open(mFileName, std::ios::app);
        os << mFormatter->format(level, event);
        os.close();
    }
}

// Logger
Logger::ptr& Logger::getInstance() {
    static Logger::ptr log(new Logger) ;
    return log;
}

void Logger::log(LogLevel::Level level, const LogEvent::ptr& event) {
    LockGuard lock(mLock);
    if (mAppends.empty()) {
        mAppends.insert(
                    std::make_pair("stdout", LogAppender::ptr(new StdoutAppender)));
    }
    for (const auto& it : mAppends) {
        it.second->log(level, event);
    }
}

void Logger::addAppender(const string filename, LogLevel::Level level) {
    LockGuard lock(mLock);
    if (mAppends.find(filename) == mAppends.end()) {
        if (filename == "stdout") {
            mAppends.insert(
                std::make_pair("stdout", LogAppender::ptr(new StdoutAppender)));
        } else {
            mAppends.insert(std::make_pair(
                filename, LogAppender::ptr(new FileAppender(filename))));
        }
    }
}

void Logger::delAppender(const string filename) {
    LockGuard lock(mLock);
    if (mAppends.find(filename) != mAppends.end())
        mAppends.erase(filename);
}

void Logger::setAppenderLevel(const string filename, LogLevel::Level level) {
    LockGuard lock(mLock);
    if(mAppends.find(filename) == mAppends.end()) {
        return;
    }
    mAppends[filename]->setLevel(level);
}

void Logger::setAppenderFormatter(const string filename, const string& format) {
    LockGuard lock(mLock);
    if(mAppends.find(filename) == mAppends.end()) {
        return;
    }
    mAppends[filename]->setFormatter(format);
}


}

