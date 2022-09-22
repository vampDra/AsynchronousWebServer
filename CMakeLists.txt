cmake_minimum_required(VERSION 3.16)
project(server)


#添加编译参数
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -pthread")


set(SRC src/lock/lock.cpp
        src/log/logger.cpp
        src/fiber/fiber.cpp
        src/scheduler/scheduler.cpp
        src/iomanager/IOManager.cpp
        src/hook/hook.cpp
        src/fd_manager/fd_manager.cpp
        src/socket/address.cpp
        src/socket/socket.cpp
        src/tcp_server/tcp_server.cpp
        src/timer/timer.cpp
        src/bytearray/bytearray.cpp
        src/http/http.cpp
        src/http/http_parser.cpp
        src/http/http11_parser.cpp
        src/http/http_server.cpp
        src/http/http_servlet.cpp
        src/http/test_server.cpp)

add_library(server SHARED ${SRC})

set(LIBS server
         dl
         pthread)

include_directories(
        src
        src/log
        src/config
        src/lock
        src/fiber
        src/iomanager
        src/scheduler
        src/hook
        src/fd_manager
        src/socket
        src/bytearray
        src/http
        src/tcp_server
        src/timer)

add_executable(log_test src/test/log_test.cpp)
target_link_libraries(log_test ${LIBS})

add_executable(fiber_test src/test/fiber_test.cpp)
target_link_libraries(fiber_test ${LIBS})

add_executable(scheduler_test src/test/scheduler_test.cpp)
target_link_libraries(scheduler_test ${LIBS})

add_executable(iom_test src/test/iom_test.cpp)
target_link_libraries(iom_test ${LIBS})

add_executable(hook_test src/test/hook_test.cpp)
target_link_libraries(hook_test ${LIBS})

add_executable(http_parser_test src/test/http_parser_test.cpp)
target_link_libraries(http_parser_test ${LIBS})

add_executable(http_server_test src/test/http_server_test.cpp)
target_link_libraries(http_server_test ${LIBS})

add_executable(tcp_server_test src/test/tcp_server_test.cpp)
target_link_libraries(tcp_server_test ${LIBS})

add_executable(test_server src/test/test_server_test.cpp)
target_link_libraries(test_server ${LIBS})

add_executable(test1 src/test/test_file1.cpp)
target_link_libraries(test1 ${LIBS})

#设置输出路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/exe)
set(LIBRARY_OUTPUT_PATH  ${PROJECT_SOURCE_DIR}/lib)