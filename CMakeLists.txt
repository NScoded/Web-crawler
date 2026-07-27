cmake_minimum_required(VERSION 3.15)
project(crawler)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ixwebsocket REQUIRED)
find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)

# MySQL
include_directories("C:/Program Files/MySQL/MySQL Server 8.0/include")
link_directories("C:/Program Files/MySQL/MySQL Server 8.0/lib")

add_executable(crawler
    src/Crawler.cpp
    src/Frontier.cpp
    src/HtmlParser.cpp
    src/Normalizer.cpp
    src/PageStorage.cpp
    src/Indexer.cpp
    CDP/CDPScraper.cpp
)

target_link_libraries(crawler
    PRIVATE
        ixwebsocket::ixwebsocket
        CURL::libcurl
        nlohmann_json::nlohmann_json
        libmysql
)