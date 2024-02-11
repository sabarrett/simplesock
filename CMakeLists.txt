cmake_minimum_required(VERSION 3.10)

project(SimpleSock)

add_executable(SimpleSock main.cpp socklib_generic.cpp tests.cpp)

target_compile_features(SimpleSock PRIVATE cxx_std_17)

# target_compile_options(SimpleSock PRIVATE -fsanitize=address)
# target_link_options(SimpleSock PRIVATE -fsanitize=address)

set(CMAKE_BUILD_TYPE Debug)

if (UNIX)
	target_sources(SimpleSock PRIVATE socklib_posix.cpp)
endif (UNIX)

if (WIN32)
	target_sources(SimpleSock PRIVATE socklib_win32.cpp)
endif (WIN32)

target_sources(SimpleSock PRIVATE pool.cpp)
