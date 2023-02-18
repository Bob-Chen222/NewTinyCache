cmake_minimum_required(VERSION 3.25)

project(cache)

set (CMAKE_CXX_STANDARD 11)

add_executable(${PROJECT_NAME} main.cpp)

find_package(TBB)

target_link_libraries(cache TBB::tbb)




