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
```
./main [-p xxx]
```
默认端口号为10000, 可通过-p指定端口
