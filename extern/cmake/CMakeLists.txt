project(stb LANGUAGES CXX)

add_library(stb INTERFACE)

get_filename_component(STB_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/../stb" ABSOLUTE)

target_include_directories(stb INTERFACE
    ${STB_INCLUDE_DIR}
)

add_library(stb::stb ALIAS stb)