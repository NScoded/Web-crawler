cmake_minimum_required(VERSION 3.16)

project(Crawler)

set(CMAKE_CXX_STANDARD 17)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SQLITE3 REQUIRED sqlite3)

include_directories(
    include
    ${SQLITE3_INCLUDE_DIRS}
)

add_executable(Crawler
    main.cpp
    src/Database.cpp
)

target_link_libraries(Crawler
    ${SQLITE3_LIBRARIES}
)