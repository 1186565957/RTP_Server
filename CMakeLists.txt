#CMakeLists.txt
set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -00 -Wall -g2 -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -03 -Wall")

cmake_minimum_required(VERSION 2.8)

PROJECT(RTPServer)
aux_source_directory(. SRC_LIST)
add_executable(main ${SRC_LIST})
