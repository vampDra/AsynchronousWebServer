# C++ 高性能后台服务器

## Introduction
- 本项目采用modern c++实现Linux下高性能后台服务器，通过`ucontext_t`自封装协程达到以写同步的方式获得异步socket的性能。
- 项目包含日志系统、封装协程模块、协程调度模块、hook异步IO模块、定时器模块、服务器模块以及`http`等主要模块。
- 项目参考了 [sylar-yin/sylar](https://github.com/sylar-yin/sylar) 并修复其多线程读写时由于协程切换bug出现的core dump.
- http协议解析采用的是 [nodejs/http-parser](https://github.com/nodejs/http-parser)

## Technical points(详细介绍见文件：模块详解)
- 主并发模型为多线程Proactor，采用eventfd线程间通信。
- 采用linux下ucontext_t结构体封装协程(Fiber模块)。
- 设计协程调度器，采用非对称式设计实现协程切换(子协程仅能与主协程切换)，实现 线程 N：M 协程 调度(scheduler模块与iomanager模块)。
- epoll_wait事件触发配合协程hook原始socket IO，以达到异步效果(hook模块与fd_manager模块)。
- 封装Tcp_server，上层协议服务器可通过继承Tcp_server轻易实现(例如http_server)
- 采用单层时间轮定时器配合epoll_wait超时参数实现毫秒级定时(timer模块)。
- 单例模式实现流式日志系统，支持日志格式解析与标准输出、文件输出两种输出地设置(logger模块)。

## Build
```
mkdir build
cd build
cmake ..
make
```

## Usage
```
./main [-p port] [-t thread_cnt]
```

## 压测结果
机器： i5-7300hq(4核心) 16G

测试： 
- 通过ab压测 100万请求 200负载 4线程处理。

  - 长连接： 66000+ QPS

  - 短连接： 25000+ QPS

- webbench测试: 5秒 200负载

  - 14.2w / 5 QPS

长连接：

![图片](https://github.com/vampDra/multi-thread-Server/blob/main/%E5%8E%8B%E6%B5%8B%E7%BB%93%E6%9E%9C/long_connection.jpg)

短连接：

![图片](https://github.com/vampDra/multi-thread-Server/blob/main/%E5%8E%8B%E6%B5%8B%E7%BB%93%E6%9E%9C/short_connection.jpg)

Webbench:

![图片](https://github.com/vampDra/multi-thread-Server/blob/main/%E5%8E%8B%E6%B5%8B%E7%BB%93%E6%9E%9C/webbench.jpg)
