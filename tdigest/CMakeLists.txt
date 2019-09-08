cmake_minimum_required (VERSION 3.11)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(rtstat_tdigest tdigest.cpp)