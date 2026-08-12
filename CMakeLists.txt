cmake_minimum_required(VERSION 3.15)
project(crawler)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ==========================================================
# Generate executables in the project root directory
# ==========================================================
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_SOURCE_DIR})

find_package(ixwebsocket REQUIRED)
find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)

# MySQL
include_directories("C:/Program Files/MySQL/MySQL Server 8.0/include")
link_directories("C:/Program Files/MySQL/MySQL Server 8.0/lib")

# ==========================================================
# crawler.exe
# ==========================================================
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

# ==========================================================
# searchengine.exe
# ==========================================================
add_executable(searchengine
    src/SearchEngine.cpp
    src/Frontier.cpp
    src/HtmlParser.cpp
    src/Normalizer.cpp
    src/PageStorage.cpp
    src/Indexer.cpp
    CDP/CDPScraper.cpp
)

target_link_libraries(searchengine
    PRIVATE
        ixwebsocket::ixwebsocket
        CURL::libcurl
        nlohmann_json::nlohmann_json
        libmysql
        ws2_32
)
