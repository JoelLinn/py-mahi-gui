# MIT License
#
# Copyright (c) 2020 Mechatronics and Haptic Interfaces Lab - Rice University
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# Author(s): Evan Pezent (epezent@rice.edu), Joel Linn

import mahi_gui
from mahi_gui import imgui
from mahi_gui import implot
import numpy as np
from random import random

class PlotItem():
    data_x = np.array([])
    data_y = np.array([])
    color = imgui.Vec4()
    label = ""


class PlotBench(mahi_gui.Application):
    POINTS = 1000
    PLOTS = 100

    def __init__(self):
        self.items = []
        self.render = imgui.Bool(True)
        self.animate  = imgui.Bool(False)
        super(PlotBench, self).__init__(500, 500, "Python Plots Benchmark")

        imgui.get_io().ini_filename = None
        self.set_vsync(False)
        imgui.disable_viewports()
        for i in range(self.PLOTS):
            item = PlotItem()
            item.data_x = np.empty(self.POINTS, dtype="uint16")
            item.data_y = np.empty(self.POINTS)
            item.color = self.random_color()
            item.label = "item_{}".format(i)
            self.items.append(item)
        self.generate_items_data()

    def random_color(self):
        return imgui.Vec4(random(), random(), random(), 1)

    def generate_items_data(self):
        for i in range(len(self.items)):
            item = self.items[i]
            y = i * 0.01
            for j in range(item.data_x.shape[0]):
                item.data_x[j] = j
                item.data_y[j] = y + (random() / 50 - 0.01)

    def _update(self):
        if (self.animate.value):
            self.generate_items_data()

        width, height = self.get_window_size()
        imgui.begin(
            "Python Plot Benchmark",
            imgui.Bool(True),
            imgui.WindowFlags.NoTitleBar | imgui.WindowFlags.NoResize | imgui.WindowFlags.NoMove)
        imgui.set_window_pos(imgui.Vec2(0,0))
        imgui.set_window_size(imgui.Vec2(width, height))
        if(imgui.button("VSync On")):
            self.set_vsync(True)
        imgui.same_line()
        if(imgui.button("VSync Off")):
            self.set_vsync(False)
        imgui.same_line()
        imgui.checkbox("Render", self.render)
        imgui.same_line()
        imgui.checkbox("Animate", self.animate)
        imgui.text("{} lines, {} pts ea. @ {:.3f} FPS".format(self.PLOTS, self.POINTS, imgui.get_io().framerate))
        implot.set_next_plot_limits_x(0, self.POINTS)
        if (implot.begin_plot("##Plot", None, None, imgui.Vec2(-1, -1), implot.Flags.NoChild)):
            if (self.render.value):
                for item in self.items:
                    implot.push_style_color(implot.Color.Line, item.color)
                    implot.plot_line(item.label, item.data_x, item.data_y)
                    implot.pop_style_color(implot.Color.Line)
            implot.end_plot()
        imgui.end()

if __name__ == "__main__":
    app = PlotBench()
    app.run()
