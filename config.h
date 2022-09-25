#ifndef __CONFIG_H__
#define __CONFIG_H__
#include <cstring>
#include <unistd.h>
#include <iostream>
using namespace std;

class Config {
public:
    Config (int port, int threadCnt);
    void parse_arg(int argc, char*argv[]);
    int mPort;
    int mThreadCnt;
};

#endif