cmake_minimum_required(VERSION 3.10)

project(SimpleSock)

add_executable(SimpleSock main.cpp)

target_compile_features(SimpleSock PRIVATE cxx_std_17)

set(CMAKE_BUILD_TYPE Debug)

if (UNIX)
	target_sources(SimpleSock PRIVATE socklib_posix.cpp)
endif (UNIX)

if (WIN32)
	target_sources(SimpleSock PRIVATE socklib_win32.cpp)
endif (WIN32)

target_sources(SimpleSock PRIVATE allocators.cpp)
