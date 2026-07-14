cmake_minimum_required(VERSION 3.15)
project(cdp_scraper)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)
find_package(ixwebsocket REQUIRED)
find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)

add_executable(cdp_scraper
    CDPScraper.cpp
    main.cpp
)

target_link_libraries(cdp_scraper
    PRIVATE
        ixwebsocket::ixwebsocket
        CURL::libcurl
        nlohmann_json::nlohmann_json
        Threads::Threads
)

# Agar iss library ko kisi doosre project/target mein use karna ho, to bas
# CDPScraper.h + CDPScraper.cpp ko us target ke sources mein add kar do,
# ya isse ek static library bana lo:
#
# add_library(cdp_scraper_lib STATIC CDPScraper.cpp)
# target_link_libraries(cdp_scraper_lib PUBLIC ixwebsocket::ixwebsocket CURL::libcurl nlohmann_json::nlohmann_json)
# target_include_directories(cdp_scraper_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
