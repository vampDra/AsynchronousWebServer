#include "config.h"

Config::Config(int port, int threadCnt)
: mPort(port)
, mThreadCnt(threadCnt) {}

void Config::parse_arg(int argc, char*argv[]){
    int opt;
    const char *str = "p:t:";
    while ((opt = getopt(argc, argv, str)) != -1)
    {
        switch (opt)
        {
        case 'p':
        {
            mPort = atoi(optarg);
            break;
        }
        case 't':
        {
            mThreadCnt = atoi(optarg);
            break;
        }
        default:
            break;
        }
    }
}