cmake_minimum_required(VERSION 3.21)
project(metal-cpp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(metal-cpp INTERFACE)

target_include_directories(metal-cpp INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(metal-cpp INTERFACE
    "-framework Metal"
    "-framework Foundation"
    "-framework QuartzCore"
)