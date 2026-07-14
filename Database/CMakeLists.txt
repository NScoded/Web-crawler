cmake_minimum_required(VERSION 3.16)

project(WebCrawler)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Crawler
    Database.cpp
)

# MySQL include path
target_include_directories(Crawler PRIVATE
    "C:/Program Files/MySQL/MySQL Server 8.0/include"
)

# MySQL library path
target_link_directories(Crawler PRIVATE
    "C:/Program Files/MySQL/MySQL Server 8.0/lib"
)

# Link MySQL client library
target_link_libraries(Crawler PRIVATE
    libmysql
)