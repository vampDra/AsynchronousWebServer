#include"scheduler.h"

server::Logger::ptr& logger = GET_LOG_INSTANCE;
void test(int i) {
    LOG_INFO(logger) << std::this_thread::get_id() << " " << i;
}

int main() {
    logger->addAppender("stdout");
    logger->setAppenderLevel("stdout", server::LogLevel::INFO);
    server::Scheduler::ptr sche(new server::Scheduler(2));
    sche->start();
    for(int i = 0; i < 10; i++) {
        sche->addTask(std::bind(test, i));
    }
    sche->stop();
}