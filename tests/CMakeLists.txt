cmake_minimum_required (VERSION 2.8.11)
project (LiveProfilerTest)

FILE(GLOB TestCases ./Cases/*/*.cpp)
add_executable(${PROJECT_NAME} ${TestCases} Main.cpp)

include_directories(../include)
set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_FLAGS "-O3 -g --std=c++14")
