cmake_minimum_required(VERSION 3.10)
project(mahi_gui)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(MAHI_UTIL_DEFAULT_LOG OFF CACHE BOOL "")

add_subdirectory(thirdparty/pybind11)
add_subdirectory(thirdparty/mahi-gui)

pybind11_add_module(mahi_gui src/imgui.cpp src/imgui_custom.cpp src/implot.cpp src/mahi_gui.cpp src/module.cpp src/imgui_helper.h src/pybind_cast.hpp)
target_link_libraries(mahi_gui PRIVATE mahi::gui)
