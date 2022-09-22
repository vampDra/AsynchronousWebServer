#include "IOManager.h"


server::Logger::ptr log = GET_LOG_INSTANCE;
void test(int i) {
    for(int i = 0; i < 1000; i++) {
    }
    server::Fiber::yieldToHold();
    // server::IOManager::getCurIOManager()->addTask(server::Fiber::getCurFiber());
    // server::Fiber::getCurFiber()->yield();
    for(int i = 0; i < 1000; i++) {
    }
    printf("%d\n", i);
}

int main() {
    log->addAppender("stdout");
    log->setAppenderLevel("stdout", server::LogLevel::INFO);
    server::IOManager iom(6);
    cout << "begin" << endl;
    for(int i = 0; i < 100000; i++) {
        iom.addTask(std::bind(test, i));
    }
    iom.stop();
}