#include"logger.h"
using namespace server;
Logger::ptr logger = GET_LOG_INSTANCE;


int main() {
    logger->addAppender("log.txt");
    logger->addAppender("stdout");
    logger->setAppenderLevel("log.txt", LogLevel::WARNING);
    LOG_DEBUG(logger) << "ddd";
    LOG_ERROR(logger) << "sss";

}