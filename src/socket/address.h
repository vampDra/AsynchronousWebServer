#ifndef __ADDRESS_H__
#define __ADDRESS_H__
#include <memory>
#include <arpa/inet.h>
#include <cstring>

namespace server {
/*
 * 广播地址是子网掩码之后的位数全为1
 * 子网掩码是前prefix全为0
 */
class Address {
public:
    typedef std::shared_ptr<Address> ptr;
    Address(std::string ip, uint16_t port = -1);
    Address(sockaddr_in addr);
    Address() = default;
    ~Address();
    Address::ptr broadcastAddress(uint32_t prefix);         //获取广播地址
    Address::ptr networdAddress(uint32_t prefix);           //获取网段地址
    Address::ptr subnetAddress(uint32_t prefix);            //获取子网掩码地址

    std::string getDottedDecimalIP() const;                 //获取点分十进制IP
    sockaddr*   getAddr() const;                            //获取sockaddr
    void setPort(uint16_t port);                            //设置端口
    int  getPort() const;                                   //获取端口
    int  getFamily() const;                                 //获取协议
    int  getAddrLen() const;                                //获取地址长度
private:
    uint32_t createSubNetMask(uint32_t prefix);             //创建二进制子网掩码(小端序)
private:
    sockaddr_in mAddr;
};


}

#endif