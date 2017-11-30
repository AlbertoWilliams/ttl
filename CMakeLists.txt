cmake_minimum_required(VERSION 3.0)
PROJECT(scorer LANGUAGES CXX VERSION 1.0)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++0x")

aux_source_directory(. SRCS)
add_executable(ttlc ${SRCS})
target_link_libraries(ttlc readline)
