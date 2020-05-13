cmake_minimum_required(VERSION 3.10)
project(mahi_gui)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_subdirectory(pybind11)
add_subdirectory(mahi-gui)

pybind11_add_module(mahi_gui src/mahi-gui.cpp)
target_link_libraries(mahi_gui PRIVATE mahi::gui)
