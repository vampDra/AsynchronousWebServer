#include "address.h"

namespace server {
    
Address::Address(std::string ip, uint16_t port) {
    bzero(&mAddr, sizeof mAddr);
    mAddr.sin_family = AF_INET;
    if(port >= 0) {
        mAddr.sin_port = htons(port);
    }
    if(ip == "0") {
        mAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    } else {
        mAddr.sin_addr.s_addr = inet_addr(ip.c_str());
    }
}

Address::Address(sockaddr_in addr) {
    mAddr = addr;
}

Address::~Address() {
    memset(&mAddr, 0, sizeof mAddr);
}

uint32_t Address::createSubNetMask(uint32_t prefix) {
    return ~((1 << (32 - prefix)) - 1);
}

Address::ptr Address::broadcastAddress(uint32_t prefix) {
    if(prefix > 32)
        return nullptr;
    sockaddr_in addr = mAddr;
    addr.sin_addr.s_addr |= htonl(~createSubNetMask(prefix));

    return Address::ptr(new Address(addr));
}

Address::ptr Address::networdAddress(uint32_t prefix) {
    if(prefix > 32)
        return nullptr;
    sockaddr_in addr = mAddr;
    addr.sin_addr.s_addr &= htonl(~createSubNetMask(prefix));
    return Address::ptr (new Address(addr));
}

Address::ptr Address::subnetAddress(uint32_t prefix) {
    if(prefix > 32)
        return nullptr;
    sockaddr_in addr = mAddr;
    addr.sin_addr.s_addr = htonl(createSubNetMask(prefix));
    return Address::ptr(new Address(addr));
}

sockaddr *Address::getAddr() const {
    return (sockaddr*) &mAddr;
}

std::string Address::getDottedDecimalIP() const {
    char buf[20];
    return inet_ntop(AF_INET, &mAddr.sin_addr.s_addr, buf, sizeof buf);
}

int Address::getFamily() const {
    return mAddr.sin_family;
}

int Address::getAddrLen() const {
    return sizeof mAddr;
}

int Address::getPort() const {
    return ntohs(mAddr.sin_port);
}

void Address::setPort(uint16_t port) {
    mAddr.sin_port = htons(port);
}



}