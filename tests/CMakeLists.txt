cmake_minimum_required (VERSION 2.8.11)
project (LiveProfilerTest)

FILE(GLOB_RECURSE TestCases ./Cases/*.cpp)
add_executable(${PROJECT_NAME} ${TestCases} Main.cpp)

# include_directories just be ignored on windows, without a fucking damn reason
# include_directories(../include)
set(CMAKE_VERBOSE_MAKEFILE TRUE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I../../include")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O3 -g --std=c++14 -fno-omit-frame-pointer")

if (WIN32)
	target_link_libraries(${PROJECT_NAME} pthread)
elseif (UNIX)
	target_link_libraries(${PROJECT_NAME} pthread bfd)
endif ()
