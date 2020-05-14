cmake_minimum_required(VERSION 3.10)
project(mahi_gui)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_subdirectory(pybind11)
add_subdirectory(mahi-gui)

pybind11_add_module(mahi_gui src/imgui.cpp src/mahi-gui.cpp src/pybind-cast.hpp)
target_link_libraries(mahi_gui PRIVATE mahi::gui)
