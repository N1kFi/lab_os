cmake_minimum_required(VERSION 3.10)
project(BullsAndCows)

set(CMAKE_CXX_STANDARD 17)

find_package(PkgConfig REQUIRED)
pkg_check_modules(ZMQ REQUIRED libzmq)

add_executable(server server.cpp)
add_executable(client client.cpp)

# libzmq
target_include_directories(server PRIVATE
        ${ZMQ_INCLUDE_DIRS}
        /opt/homebrew/include
)

target_include_directories(client PRIVATE
        ${ZMQ_INCLUDE_DIRS}
        /opt/homebrew/include
)

# Пути к библиотекам
target_link_directories(server PRIVATE /opt/homebrew/lib)
target_link_directories(client PRIVATE /opt/homebrew/lib)

# Линковка
target_link_libraries(server zmq)
target_link_libraries(client zmq)
