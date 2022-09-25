# multi-thread-Server
c++编写的Linux多线程服务器，通过ucontext_t封装协程，并通过协程封装异步socket。


## 构建项目
```
mkdir build
cd build
cmake ..
make
```
最后生成的可执行文件在 ../bin/main


## 使用说明
默认端口号为10000， 线程数为4
```
./main [-p port] [-t thread_cnt]
```


## 压测结果
机器： i5-7300hq(4核心) 16G

测试： 通过ab压测 10万请求 200负载 4线程处理。

长连接： 59000+ QPS

![图片](https://github.com/vampDra/multi-thread-Server/blob/main/%E5%8E%8B%E6%B5%8B%E7%BB%93%E6%9E%9C/long_connect.jpg)

短连接： 15000 QPS

![图片](https://github.com/vampDra/multi-thread-Server/blob/main/%E5%8E%8B%E6%B5%8B%E7%BB%93%E6%9E%9C/short_connect.jpg)
