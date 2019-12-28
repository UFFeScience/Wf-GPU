cmake_minimum_required(VERSION 2.8.9)
project(Static-HEA)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3  -Wall -std=c++11")
# bring the headers into the project
include_directories(include)

#add_executable(HEA ./source/hea.cpp)
add_executable(HEA /mnt/d/mestrado/Wf-GPU/gpu-heuristic/source/hea.cpp)
#add_executable(HEA /home/murilostockinger/Mestrado/Wf-GPU/gpu-heuristic/source/hea.cpp)
