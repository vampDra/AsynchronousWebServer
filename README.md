# multi-thread-Server
Linux WebServer，enclosure fiber by ucontext_t and realize Asynchronous IO via fiber.


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
