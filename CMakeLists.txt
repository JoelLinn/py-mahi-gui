cmake_minimum_required(VERSION 3.10)
project(mahi_gui)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Do not write log file
set(MAHI_UTIL_DEFAULT_LOG OFF CACHE BOOL "")
# Do not link against GTK+ 3
set(MAHI_GUI_NATIVE_DIAGS OFF CACHE BOOL "")

add_subdirectory(thirdparty/mahi-gui)
add_subdirectory(thirdparty/pybind11)

set(MAHI_GUI_HEADERS
        src/imgui_helper.h
        src/leaked_ptr.h
        src/pybind_cast.hpp
        )

set(MAHI_GUI_SRC
        src/imgui.cpp
        src/imgui_custom.cpp
        src/implot.cpp
        src/mahi_gui.cpp
        src/module.cpp
        )

pybind11_add_module(mahi_gui ${MAHI_GUI_SRC} ${MAHI_GUI_HEADERS})
target_link_libraries(mahi_gui PRIVATE mahi::gui)
