cmake_minimum_required (VERSION 3.11)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project (rtstat)

include_directories ("${PROJECT_SOURCE_DIR}/p2")
include_directories ("${PROJECT_SOURCE_DIR}/tdigest")

add_subdirectory(p2)
add_subdirectory(tdigest)

add_executable(rtstat test.cpp)
target_link_libraries (rtstat rtstat_p2 rtstat_tdigest)

