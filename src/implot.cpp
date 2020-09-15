/******************************************************************************

Copyright 2020 Joel Linn

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

You are under no obligation whatsoever to provide any bug fixes, patches, or
upgrades to the features, functionality or performance of the source code
("Enhancements") to anyone; however, if you choose to make your Enhancements
available either publicly, or directly to the author of this software, without
imposing a separate written license agreement for such Enhancements, then you
hereby grant the following license: a non-exclusive, royalty-free perpetual
license to install, use, modify, prepare derivative works, incorporate into
other computer software, distribute, and sublicense such enhancements or
derivative works thereof, in binary and source code form.

******************************************************************************/

#include <implot.h>
#include <pybind11/pybind11.h>

#include "imgui_helper.h"

namespace py = pybind11;

#define PY_BUF_IS_TYPE(__type__, __buffer_info__)                              \
  (py::detail::compare_buffer_info<__type__>::compare(__buffer_info__))

// RAII Helper to pin buffers and template expand correct callback getter func.
struct ValueGetter {
public:
  ValueGetter(const py::buffer& bufY) : hasX(false), infoY(bufY.request()) {
    if (this->infoY.ndim != 1) {
      throw std::runtime_error(error_dim);
    }
  }
  ValueGetter(const py::buffer& bufX, const py::buffer& bufY)
      : hasX(true), infoX(bufX.request()), infoY(bufY.request()) {
    if (this->infoX.ndim != 1 || this->infoY.ndim != 1 ||
        this->infoX.shape.at(0) != this->infoY.shape.at(0)) {
      throw std::runtime_error(error_dim);
    }
  }

  typedef ImPlotPoint getter_func(void* data, int idx);
  getter_func* get_getter_func() const {
#define VG_EMIT_GET_GETTER_Y(__type__)                                         \
  if (PY_BUF_IS_TYPE(__type__, this->infoX)) {                                 \
    return get_getter_func_y<__type__>();                                      \
  }

    if (hasX) {
      VG_EMIT_GET_GETTER_Y(bool);
      VG_EMIT_GET_GETTER_Y(float);
      VG_EMIT_GET_GETTER_Y(double);
      VG_EMIT_GET_GETTER_Y(int8_t);
      VG_EMIT_GET_GETTER_Y(uint8_t);
      VG_EMIT_GET_GETTER_Y(int16_t);
      VG_EMIT_GET_GETTER_Y(uint16_t);
      VG_EMIT_GET_GETTER_Y(int32_t);
      VG_EMIT_GET_GETTER_Y(uint32_t);
      VG_EMIT_GET_GETTER_Y(int64_t);
      VG_EMIT_GET_GETTER_Y(uint64_t);
      throw std::runtime_error(error_type);
    } else {
      return get_getter_func_y<void>();
    }
#undef VG_EMIT_GET_GETTER_Y
  }

  int count() const { return this->infoY.shape.at(0); };

protected:
  static const constexpr char* error_dim = "Incompatible buffer dimension!";
  static const constexpr char* error_type =
      "Incompatible format: expected array of bool, float, double or "
      "unsigned/signed int 8, 16, 32 or 64!";

  template <typename X, typename Y>
  static ImPlotPoint getValue(void* data, int idx) {
    const auto* this_ = static_cast<ValueGetter*>(data);
    double x, y;
    if constexpr (std::is_void<X>::value) {
      x = static_cast<double>(idx);
    } else {
      x = static_cast<double>((static_cast<X*>(this_->infoX.ptr))[idx]);
    }
    y = static_cast<double>((static_cast<Y*>(this_->infoY.ptr))[idx]);
    return ImPlotPoint(x, y);
  }

  template <typename X> inline getter_func* get_getter_func_y() const {
#define VG_EMIT_RET_GETTER(__type__)                                           \
  if (PY_BUF_IS_TYPE(__type__, this->infoY)) {                                 \
    return &getValue<X, __type__>;                                             \
  }

    VG_EMIT_RET_GETTER(bool);
    VG_EMIT_RET_GETTER(float);
    VG_EMIT_RET_GETTER(double);
    VG_EMIT_RET_GETTER(int8_t);
    VG_EMIT_RET_GETTER(uint8_t);
    VG_EMIT_RET_GETTER(int16_t);
    VG_EMIT_RET_GETTER(uint16_t);
    VG_EMIT_RET_GETTER(int32_t);
    VG_EMIT_RET_GETTER(uint32_t);
    VG_EMIT_RET_GETTER(int64_t);
    VG_EMIT_RET_GETTER(uint64_t);
    throw std::runtime_error(error_type);
#undef VG_EMIT_RET_GETTER
  }

private:
  const bool hasX;
  const py::buffer_info infoX;
  const py::buffer_info infoY;
};

void py_init_module_implot(py::module& m) {

  py::enum_<ImPlotFlags_>(m, "Flags", py::arithmetic())
      .value("MousePos", ImPlotFlags_MousePos)
      .value("Legend", ImPlotFlags_Legend)
      .value("Highlight", ImPlotFlags_Highlight)
      .value("BoxSelect", ImPlotFlags_BoxSelect)
      .value("Query", ImPlotFlags_Query)
      .value("ContextMenu", ImPlotFlags_ContextMenu)
      .value("Crosshairs", ImPlotFlags_Crosshairs)
      .value("AntiAliased", ImPlotFlags_AntiAliased)
      .value("NoChild", ImPlotFlags_NoChild)
      .value("YAxis2", ImPlotFlags_YAxis2)
      .value("YAxis3", ImPlotFlags_YAxis3)
      .value("Default", ImPlotFlags_Default);

  py::enum_<ImPlotAxisFlags_>(m, "AxisFlags", py::arithmetic())
      .value("GridLines", ImPlotAxisFlags_GridLines)
      .value("TickMarks", ImPlotAxisFlags_TickMarks)
      .value("TickLabels", ImPlotAxisFlags_TickLabels)
      .value("Invert", ImPlotAxisFlags_Invert)
      .value("LockMin", ImPlotAxisFlags_LockMin)
      .value("LockMax", ImPlotAxisFlags_LockMax)
      .value("LogScale", ImPlotAxisFlags_LogScale)
      .value("Scientific", ImPlotAxisFlags_Scientific)
      .value("Default", ImPlotAxisFlags_Default)
      .value("Auxiliary", ImPlotAxisFlags_Auxiliary);

  py::enum_<ImPlotCol_>(m, "Color")
      .value("Line", ImPlotCol_Line)
      .value("Fill", ImPlotCol_Fill)
      .value("MarkerOutline", ImPlotCol_MarkerOutline)
      .value("MarkerFill", ImPlotCol_MarkerFill)
      .value("ErrorBar", ImPlotCol_ErrorBar)
      .value("FrameBg", ImPlotCol_FrameBg)
      .value("PlotBg", ImPlotCol_PlotBg)
      .value("PlotBorder", ImPlotCol_PlotBorder)
      .value("XAxis", ImPlotCol_XAxis)
      .value("YAxis", ImPlotCol_YAxis)
      .value("YAxis2", ImPlotCol_YAxis2)
      .value("YAxis3", ImPlotCol_YAxis3)
      .value("Selection", ImPlotCol_Selection)
      .value("Query", ImPlotCol_Query);

  py::enum_<ImPlotStyleVar_>(m, "StyleVar")
      .value("LineWeight", ImPlotStyleVar_LineWeight)
      .value("Marker", ImPlotStyleVar_Marker)
      .value("MarkerSize", ImPlotStyleVar_MarkerSize)
      .value("MarkerWeight", ImPlotStyleVar_MarkerWeight)
      .value("FillAlpha", ImPlotStyleVar_FillAlpha)
      .value("ErrorBarSize", ImPlotStyleVar_ErrorBarSize)
      .value("ErrorBarWeight", ImPlotStyleVar_ErrorBarWeight)
      .value("DigitalBitHeight", ImPlotStyleVar_DigitalBitHeight)
      .value("DigitalBitGap", ImPlotStyleVar_DigitalBitGap);

  py::enum_<ImPlotMarker_>(m, "Marker")
      .value("None_", ImPlotMarker_None)
      .value("Circle", ImPlotMarker_Circle)
      .value("Square", ImPlotMarker_Square)
      .value("Diamond", ImPlotMarker_Diamond)
      .value("Up", ImPlotMarker_Up)
      .value("Down", ImPlotMarker_Down)
      .value("Left", ImPlotMarker_Left)
      .value("Right", ImPlotMarker_Right)
      .value("Cross", ImPlotMarker_Cross)
      .value("Plus", ImPlotMarker_Plus)
      .value("Asterisk", ImPlotMarker_Asterisk);

  py::enum_<ImPlotColormap_>(m, "Colormap")
      .value("Default", ImPlotColormap_Default)
      .value("Dark", ImPlotColormap_Dark)
      .value("Pastel", ImPlotColormap_Pastel)
      .value("Paired", ImPlotColormap_Paired)
      .value("Viridis", ImPlotColormap_Viridis)
      .value("Plasma", ImPlotColormap_Plasma)
      .value("Hot", ImPlotColormap_Hot)
      .value("Cool", ImPlotColormap_Cool)
      .value("Pink", ImPlotColormap_Pink)
      .value("Jet", ImPlotColormap_Jet);

  py::class_<ImPlotPoint>(m, "Point")
      .def(py::init<>())
      .def(py::init<double, double>())
      .def_readwrite("x", &ImPlotPoint::x)
      .def_readwrite("y", &ImPlotPoint::y);

  py::class_<ImPlotRange>(m, "Range")
      .def(py::init<>())
      .def_readwrite("min", &ImPlotRange::Min)
      .def_readwrite("max", &ImPlotRange::Max)
      .def("contains", &ImPlotRange::Contains, py::arg("value"))
      .def("size", &ImPlotRange::Size);

  py::class_<ImPlotLimits>(m, "Limits")
      .def(py::init<>())
      .def_readwrite("x", &ImPlotLimits::X)
      .def_readwrite("y", &ImPlotLimits::Y)
      .def(
          "contains",
          [](ImPlotLimits& self, ImPlotPoint& p) { return self.Contains(p); },
          py::arg("p"))
      .def(
          "contains",
          [](ImPlotLimits& self, double x, double y) {
            return self.Contains(x, y);
          },
          py::arg("x"), py::arg("y"));

  py::class_<ImPlotStyle>(m, "Style")
      .def(py::init())
      .def_readwrite("line_weight", &ImPlotStyle::LineWeight)
      .def_readwrite("marker", &ImPlotStyle::Marker)
      .def_readwrite("marker_size", &ImPlotStyle::MarkerSize)
      .def_readwrite("marker_weight", &ImPlotStyle::MarkerWeight)
      .def_readwrite("fill_alpha", &ImPlotStyle::FillAlpha)
      .def_readwrite("error_bar_size", &ImPlotStyle::ErrorBarSize)
      .def_readwrite("error_bar_weight", &ImPlotStyle::ErrorBarWeight)
      .def_readwrite("digital_bit_height", &ImPlotStyle::DigitalBitHeight)
      .def_readwrite("digital_bit_gap", &ImPlotStyle::DigitalBitGap)
      .def("get_color",
           [](ImPlotStyle& self, ImPlotCol idx) {
             if (idx < 0 || idx >= ImPlotCol_COUNT) {
               throw std::out_of_range("Illegal implot.Col value.");
             }
             return self.Colors[idx];
           })
      .def("set_color", [](ImPlotStyle& self, ImPlotCol idx, ImVec4 c) {
        if (idx < 0 || idx >= ImPlotCol_COUNT) {
          throw std::out_of_range("Illegal implot.Col value.");
        }
        self.Colors[idx] = c;
      });

  // ImPlotInputMap is not available for now.

  m.def("begin_plot", &ImPlot::BeginPlot, py::arg("title_id"),
        py::arg("x_label") = nullptr, py::arg("y_label") = nullptr,
        py::arg("size") = ImVec2(-1, 0), py::arg("flags") = ImPlotFlags_Default,
        py::arg("x_flags") = ImPlotAxisFlags_Default,
        py::arg("y_flags") = ImPlotAxisFlags_Default,
        py::arg("y2_flags") = ImPlotAxisFlags_Auxiliary,
        py::arg("y3_flags") = ImPlotAxisFlags_Auxiliary);
  m.def("end_plot", &ImPlot::EndPlot);

  //---------------------------------------------------------------------------
  // Plot Items
  //---------------------------------------------------------------------------

  m.def(
      "plot_line",
      [](const char* label_id, py::buffer values) {
        auto value_getter = ValueGetter(values);
        py::gil_scoped_release release;
        ImPlot::PlotLine(label_id, value_getter.get_getter_func(),
                         &value_getter, value_getter.count());
      },
      py::arg("label_id"), py::arg("values"));
  m.def(
      "plot_line",
      [](const char* label_id, py::buffer xs, py::buffer ys) {
        auto value_getter = ValueGetter(xs, ys);
        py::gil_scoped_release release;
        ImPlot::PlotLine(label_id, value_getter.get_getter_func(),
                         &value_getter, value_getter.count());
      },
      py::arg("label_id"), py::arg("xs"), py::arg("ys"));

  m.def(
      "plot_scatter",
      [](const char* label_id, py::buffer values) {
        auto value_getter = ValueGetter(values);
        py::gil_scoped_release release;
        ImPlot::PlotScatter(label_id, value_getter.get_getter_func(),
                            &value_getter, value_getter.count());
      },
      py::arg("label_id"), py::arg("values"));
  m.def(
      "plot_scatter",
      [](const char* label_id, py::buffer xs, py::buffer ys) {
        auto value_getter = ValueGetter(xs, ys);
        py::gil_scoped_release release;
        ImPlot::PlotScatter(label_id, value_getter.get_getter_func(),
                            &value_getter, value_getter.count());
      },
      py::arg("label_id"), py::arg("xs"), py::arg("ys"));

  m.def(
      "plot_shaded",
      [](const char* label_id, py::buffer xs, py::buffer ys1, py::buffer ys2) {
        // TODO
        throw std::runtime_error("not implemented");
      },
      py::arg("label_id"), py::arg("xs"), py::arg("ys1"), py::arg("ys2"));
  m.def(
      "plot_shaded",
      [](const char* label_id, py::buffer xs, py::buffer ys, double y_ref) {
        // TODO
        throw std::runtime_error("not implemented");
      },
      py::arg("label_id"), py::arg("xs"), py::arg("ys"),
      py::arg("y_ref") = 0.0);

  m.def(
      "plot_bars",
      [](const char* label_id, py::buffer values, double width, double shift) {
        auto value_getter = ValueGetter(values);
        py::gil_scoped_release release;
        ImPlot::PlotBars(label_id, value_getter.get_getter_func(),
                         &value_getter, value_getter.count(), width, shift);
      },
      py::arg("label_id"), py::arg("values"), py::arg("width") = 0.67,
      py::arg("shift") = 0.0);
  m.def(
      "plot_bars",
      [](const char* label_id, py::buffer xs, py::buffer ys, float height) {
        auto value_getter = ValueGetter(xs, ys);
        py::gil_scoped_release release;
        ImPlot::PlotBars(label_id, value_getter.get_getter_func(),
                         &value_getter, value_getter.count(), height);
      },
      py::arg("label_id"), py::arg("xs"), py::arg("ys"), py::arg("width"));

  m.def(
      "plot_bars_h",
      [](const char* label_id, py::buffer values, double height, double shift) {
        auto value_getter = ValueGetter(values);
        py::gil_scoped_release release;
        ImPlot::PlotBarsH(label_id, value_getter.get_getter_func(),
                          &value_getter, value_getter.count(), height, shift);
      },
      py::arg("label_id"), py::arg("values"), py::arg("height") = 0.67,
      py::arg("shift") = 0.0);
  m.def(
      "plot_bars_h",
      [](const char* label_id, py::buffer xs, py::buffer ys, double height) {
        auto value_getter = ValueGetter(xs, ys);
        py::gil_scoped_release release;
        ImPlot::PlotBarsH(label_id, value_getter.get_getter_func(),
                          &value_getter, value_getter.count(), height);
      },
      py::arg("label_id"), py::arg("xs"), py::arg("ys"), py::arg("height"));

  m.def(
      "plot_error_bars",
      [](const char* label_id, py::buffer xs, py::buffer ys, py::buffer neg,
         py::buffer pos) {
        // TODO implement ValueGetter for 4 components
        throw std::runtime_error("not implemented");
      },
      py::arg("label_id"), py::arg("xs"), py::arg("ys"), py::arg("neg"),
      py::arg("pos"));

  m.def(
      "plot_pie_chart",
      [](std::vector<const char*> label_ids, py::buffer values, double x,
         double y, double radius, bool normalize, const char* label_fmt,
         double angle0) {
        // TODO ImPlot::PlotPieChart has no getter overload
        throw std::runtime_error("not implemented");
      },
      py::arg("label_ids"), py::arg("values"), py::arg("x"), py::arg("y"),
      py::arg("radius"), py::arg("normalize") = false,
      py::arg("label_fmt") = "%.1f", py::arg("angle0") = 90);

  m.def("plot_heatmap", []() {
    // TODO
    throw std::runtime_error("not implemented");
  });

  m.def(
      "plot_digital",
      [](const char* label_id, py::buffer xs, py::buffer ys) {
        auto value_getter = ValueGetter(xs, ys);
        py::gil_scoped_release release;
        ImPlot::PlotDigital(label_id, value_getter.get_getter_func(),
                            &value_getter, value_getter.count());
      },
      py::arg("label_id"), py::arg("xs"), py::arg("ys"));

  m.def("plot_text",
        py::overload_cast<const char*, double, double, bool, const ImVec2&>(
            &ImPlot::PlotText),
        py::arg("text"), py::arg("x"), py::arg("y"),
        py::arg("vertical") = false, py::arg("pixel_offset") = ImVec2(0, 0));

  //---------------------------------------------------------------------------
  // Plot Queries
  //---------------------------------------------------------------------------

  m.def("is_plot_hovered", &ImPlot::IsPlotHovered);
  m.def("get_plot_mouse_pos", &ImPlot::GetPlotMousePos, py::arg("y_axis") = -1);
  m.def("get_plot_limits", &ImPlot::GetPlotLimits, py::arg("y_axis") = -1);
  m.def("is_plot_queried", &ImPlot::IsPlotQueried);
  m.def("get_plot_query", &ImPlot::GetPlotQuery, py::arg("y_axis") = -1);

  //----------------------------------------------------------------------------
  // Plot Input Mapping
  //----------------------------------------------------------------------------

  // TODO

  //----------------------------------------------------------------------------
  // Plot Styling
  //----------------------------------------------------------------------------

  m.def("get_style", &ImPlot::GetStyle);

  m.def("push_style_color",
        py::overload_cast<ImPlotCol, const ImVec4&>(&ImPlot::PushStyleColor),
        py::arg("idx"), py::arg("col"));
  m.def("push_style_color",
        py::overload_cast<ImPlotCol, ImU32>(&ImPlot::PushStyleColor),
        py::arg("idx"), py::arg("col"));
  m.def("pop_style_color", &ImPlot::PopStyleColor, py::arg("count") = 1);

  m.def("push_style_var",
        py::overload_cast<ImPlotStyleVar, float>(&ImPlot::PushStyleVar),
        py::arg("idx"), py::arg("val"));
  m.def("push_style_var",
        py::overload_cast<ImPlotStyleVar, int>(&ImPlot::PushStyleVar),
        py::arg("idx"), py::arg("val"));
  m.def("pop_style_var", &ImPlot::PopStyleVar, py::arg("count") = 1);

  m.def(
      "set_colormap",
      [](ImPlotColormap colormap, int samples) {
        ImPlot::SetColormap(colormap, samples);
      },
      py::arg("colormap"), py::arg("samples") = 0);
  m.def(
      "set_colormap",
      [](const std::vector<ImVec4> colors) {
        ImPlot::SetColormap(colors.data(), colors.size());
      },
      py::arg("colors"));
  m.def("get_colormap_size", &ImPlot::GetColormapSize);
  m.def("get_colormap_color", &ImPlot::GetColormapColor);
  m.def("lerp_colormap", &ImPlot::LerpColormap, py::arg("t"));

  //-----------------------------------------------------------------------------
  // Plot Utils
  //-----------------------------------------------------------------------------

  m.def("set_next_plot_limits", &ImPlot::SetNextPlotLimits, py::arg("x_min"),
        py::arg("x_max"), py::arg("y_min"), py::arg("y_max"),
        py::arg("cond") = ImGuiCond_Once);
  m.def("set_next_plot_limits_x", &ImPlot::SetNextPlotLimitsX, py::arg("x_min"),
        py::arg("x_max"), py::arg("cond") = ImGuiCond_Once);
  m.def("set_next_plot_limits_y", &ImPlot::SetNextPlotLimitsX, py::arg("y_min"),
        py::arg("y_max"), py::arg("cond") = ImGuiCond_Once);

  // TODO SetNextPlotTicks (string ownership)

  m.def("set_plot_y_axis", &ImPlot::SetPlotYAxis, py::arg("y_axis"));

  m.def("get_plot_pos", &ImPlot::GetPlotPos);
  m.def("get_plot_size", &ImPlot::GetPlotSize);

  m.def("pixels_to_plot",
        py::overload_cast<const ImVec2&, int>(&ImPlot::PixelsToPlot),
        py::arg("pix"), py::arg("y_axis") = -1);
  m.def("pixels_to_plot",
        py::overload_cast<float, float, int>(&ImPlot::PixelsToPlot),
        py::arg("x"), py::arg("y"), py::arg("y_axis") = -1);

  m.def("plot_to_pixels",
        py::overload_cast<const ImPlotPoint&, int>(&ImPlot::PlotToPixels),
        py::arg("plt"), py::arg("y_axis") = -1);
  m.def("plot_to_pixels",
        py::overload_cast<double, double, int>(&ImPlot::PlotToPixels),
        py::arg("x"), py::arg("y"), py::arg("y_axis") = -1);

  m.def("push_plot_clip_rect", &ImPlot::PushPlotClipRect);
  m.def("pop_plot_clip_rect", &ImPlot::PopPlotClipRect);

  //---------------------------------------------------------------------------
  // Demo
  //---------------------------------------------------------------------------

  // m.def("show_demo_window", py::overload_cast<>(&ImPlot::ShowDemoWindow));
  // m.def("show_demo_window", []() { ImPlot::ShowDemoWindow(); });
  m.def(
      "show_demo_window",
      [](Bool& open) {
        ImPlot::ShowDemoWindow((bool*)(open.null ? nullptr : &open.value));
      },
      py::arg("open") = null);
}
