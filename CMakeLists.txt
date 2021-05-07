cmake_minimum_required(VERSION 3.10)

project(TalyteAssistant)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)

add_executable(talyte-assistant src/client.cpp)

target_include_directories(talyte-assistant PUBLIC "./libs/websocketpp")
target_include_directories(talyte-assistant PUBLIC "./libs/asio/asio/include")

if (WIN32)
    target_include_directories(talyte-assistant PUBLIC "./libs/mingw-std-threads")
    target_link_libraries(talyte-assistant wsock32 ws2_32)
else()
    find_package(Threads)
    target_link_libraries(talyte-assistant ${CMAKE_THREAD_LIBS_INIT})
endif()
