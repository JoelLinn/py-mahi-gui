cmake_minimum_required(VERSION 3.10)
project(mahi_gui)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(pybind11)
add_subdirectory(mahi-gui)

pybind11_add_module(mahi_gui src/imgui.cpp src/implot.cpp src/mahi-gui.cpp src/imgui-helper.h src/pybind-cast.hpp)
target_link_libraries(mahi_gui PRIVATE mahi::gui)
