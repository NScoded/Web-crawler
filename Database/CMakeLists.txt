cmake_minimum_required(VERSION 3.16)

project(Crawler)

set(CMAKE_CXX_STANDARD 17)

include_directories(
"C:/Program Files/MySQL/MySQL Connector C++ 9.7/include/jdbc"
)

link_directories(
"C:/Program Files/MySQL/MySQL Connector C++ 9.7/lib64/vs14"
)

add_executable(Crawler main.cpp)

target_link_libraries(Crawler mysqlcppconn)