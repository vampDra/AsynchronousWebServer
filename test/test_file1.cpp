#include "IOManager.h"


server::Logger::ptr log = GET_LOG_INSTANCE;
void test(int i) {
    for(int i = 0; i < 100; i++) {
    }
    server::IOManager::getCurIOManager()->addTask(server::Fiber::getCurFiber());
    server::Fiber::yieldToHold();
    printf("%d\n", i);
}

int main() {
    log->addAppender("stdout", server::LogLevel::INFO);
    server::IOManager iom(8);
    cout << "begin" << endl;
    for(int i = 0; i < 10000; i++) {
        iom.addTask(std::bind(test, i));
    }
    iom.stop();
}
