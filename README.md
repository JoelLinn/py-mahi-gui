# mahi-gui Python bindings

[![Build Status](https://travis-ci.com/JoelLinn/py-mahi-gui.svg?branch=master)](https://travis-ci.com/JoelLinn/py-mahi-gui) [![Build status](https://ci.appveyor.com/api/projects/status/blcthvmiryheaewo/branch/master?svg=true)](https://ci.appveyor.com/project/JoelLinn/py-mahi-gui/branch/master)

This module enables you to create [mahi-gui](https://github.com/mahilab/mahi-gui) applications with [Dear ImGui](https://github.com/ocornut/imgui) and [ImPlot](https://github.com/epezent/implot) using Python. Note that there currently are no bindings for NanoVG.

Usage is similar to mahi-gui, inherit your Application class from `mahi_gui.Application` and overwrite the methods `_update()`, `_draw_opengl()` (named `draw()` in C++ interface). To get you started have a look into the `examples` folder.

Almost all ImGui and ImPlot functions are available, please consult their documentations. You can even call OpenGL functions inside `_draw_opengl()` to draw custom stuff. If a functionality you require is not mapped feel free to request it or create a pull request.

The bindings are maintained by hand to get the best performance, especially when plotting large datasets.

## Installation

The project is available in the Python Package Index PyPi, just run the following command:

```
pip install mahi-gui
```

If you are on a platform where no pre-compiled binaries are available or want to build from source you may need to resolve missing build dependencies.
