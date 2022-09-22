#include "tcp_server.h"


namespace server {

Logger::ptr tcpserverLogger = GET_LOG_INSTANCE;

TcpServer::TcpServer(IOManager *accept, IOManager *worker)
: mAcceptor(accept), mWorker(worker), mIsStop(true) {
    mLisSock = Socket::createTcp();
}

bool server::TcpServer::bind(Address::ptr& addr) {
    while(mLisSock->bind(addr) != 0) {
        LOG_ERROR(tcpserverLogger) << "bind error" << ", errno = " << errno << ", " << strerror(errno);
        sleep(2);
    }
    if(mLisSock->listen() != 0) {
        LOG_ERROR(tcpserverLogger) << "listen error" << ", errno = " << errno << ", " << strerror(errno);
        return false;
    }
    return true;
}

void server::TcpServer::start() {
    if(!mIsStop) {
        return;
    }
    mIsStop = false;
    mAcceptor->addTask(std::bind(&TcpServer::handleAccept, shared_from_this()));
}

void server::TcpServer::stop() {
    mIsStop = true;
}

void server::TcpServer::handleAccept() {
    Socket::ptr newClient;
    while(!mIsStop) {
        newClient.reset();
        newClient = mLisSock->accept();
        if(!newClient) {
            continue;
        }
        LOG_DEBUG(tcpserverLogger) << "new connection comming";
        mWorker->addTask(std::bind(&TcpServer::handleClient, shared_from_this(), newClient));
    }
    mLisSock->close();
}

void server::TcpServer::handleClient(Socket::ptr sock) {
    while(1) {
        char buf[1024];
        memset(buf, 0, sizeof buf);
        int len = sock->recv(buf, 1024);
        if (len == 0) {
            LOG_DEBUG(tcpserverLogger) << "connection close";
            if(sock->close() == -1) {
                LOG_ERROR(tcpserverLogger) << "close error "
                    << ", errno = " << errno << ", " << strerror(errno);
            }
            break;
        }
        else if (len < 0) {
            LOG_ERROR(tcpserverLogger) << "recv error "
                                       << ", errno = " << errno << ", " << strerror(errno);
            sock->close();
            break;
        }
        else {
            ssize_t len = sock->send(buf, strlen(buf));
            if(len != (ssize_t)strlen(buf)) {
                LOG_ERROR(tcpserverLogger) << "send error "
                                           << ", errno = " << errno << ", " << strerror(errno);
                // sock->close();
            }
        }
    }
}


}