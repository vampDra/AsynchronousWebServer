#include "fiber.h"

server::Logger::ptr logger = GET_LOG_INSTANCE;
void test_1() {
    LOG_INFO(logger) << "fiber 1, first come";
    server::Fiber::yieldToHold();
    LOG_INFO(logger) << "fiber 1, first second";
}

void test_2() {
    LOG_INFO(logger) << "fiber 2, first come";
    server::Fiber::yieldToHold();
    LOG_INFO(logger) << "fiber 2, second come";
}

void test_fiber() {
    server::Fiber::ptr mainf = server::Fiber::getCurFiber();
    server::Fiber::ptr f1(new server::Fiber(std::bind(test_1)));
    server::Fiber::ptr f2(new server::Fiber(std::bind(test_2)));
    f1->resume();
    LOG_INFO(logger) << "main first";
    f1->resume();
    LOG_INFO(logger) << "main second";
    f1->resume();
    LOG_INFO(logger) << "main third";
    f2->resume();
    LOG_INFO(logger) << "main fourth";
}

int main() {
    test_fiber();
}