cmake_minimum_required(VERSION 3.14)
project(cdp_scraper CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)
find_package(CURL REQUIRED)
find_package(ixwebsocket REQUIRED)
find_package(nlohmann_json REQUIRED)

add_executable(cdp_scraper cdp_scraper.cpp)

target_link_libraries(cdp_scraper
    PRIVATE
    CURL::libcurl
    ixwebsocket::ixwebsocket
    nlohmann_json::nlohmann_json
    pthread
)
