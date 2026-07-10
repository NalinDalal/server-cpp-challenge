cmake_minimum_required(VERSION 3.10)
project(webserver CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(webserver
    webserver.cpp
    client.cpp
)

target_include_directories(webserver PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
